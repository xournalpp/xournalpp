#ifdef _WIN32

#include "util/Win32Util.h"

#include <array>
#include <cassert>
#include <iostream>
#include <string_view>
#include <vector>

// clang-format off
// (dbghelp.h depends on windows.h)
#include <windows.h>
#include <dbghelp.h>
// clang-format on

#include "util/StringUtils.h"

#include "config-debug.h"


namespace xoj::win32 {

////////////////////////////////////////////////////////////////////////////////
// Win32 std::system_error
////////////////////////////////////////////////////////////////////////////////

namespace {

auto getSystemErrorMessage(unsigned long ec) -> UniqueLocal<wchar_t> {
    constexpr unsigned long fmFlags =
            FORMAT_MESSAGE_FROM_SYSTEM |     // Use system message table as source (and error code as message ID)
            FORMAT_MESSAGE_IGNORE_INSERTS |  // Don't try to replace any (accidential) format specifiers
            FORMAT_MESSAGE_ALLOCATE_BUFFER;  // Let the OS allocate a buffer of appropriate size
    constexpr unsigned long fmLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

    UniqueLocal<wchar_t> buffer;
    [[maybe_unused]] const auto length =
            FormatMessageW(fmFlags,
                           nullptr,  // source (ignored)
                           ec,       // message ID
                           fmLangId,
                           reinterpret_cast<wchar_t*>(buffer.oref().ptr()),  // (for FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                                                             // buffer is expected to be a wchar_t**)
                           1,         // (for FORMAT_MESSAGE_ALLOCATE_BUFFER) minimum buffer size
                           nullptr);  // format args
    assert(length);

    return buffer;
}

const class: public std::error_category {
public:
    const char* name() const noexcept override { return "Win32 Error"; }

    std::error_condition default_error_condition(int ev) const noexcept override {
        std::cerr << "WARNING: win32_category::default_error_condition is NOT properly implemented!" << std::endl;
        return std::error_condition(ev, *this);
    }

    std::string message(int ec) const noexcept override {
        auto wmsg = getSystemErrorMessage(static_cast<unsigned long>(ec));
        std::wstring_view wmsg_view(wmsg.get());

        // Strip potential newline:
        if (wmsg_view.length() > 2 && *(wmsg_view.end() - 2) == '\r' && *(wmsg_view.end() - 1) == '\n') {
            wmsg_view = wmsg_view.substr(0, wmsg_view.length() - 2);
        }

        return StringCvt::u8(wmsg_view);
    }
} systemErrorWin32Category;

}  // namespace

auto win32_errc::message() const -> std::string { return win32_category().message(this->intValue()); }

auto win32_category() noexcept -> const std::error_category& { return systemErrorWin32Category; }

auto make_error_code(win32_errc ec) noexcept -> std::error_code {
    return std::error_code(ec.intValue(), win32_category());
}

auto getLastError() -> win32_errc { return win32_errc(GetLastError()); }

win32_error::win32_error(win32_errc ec, const char* whatArg): std::system_error(ec, whatArg) {}
win32_error::win32_error(unsigned long ec, const char* whatArg): std::system_error(win32_errc(ec), whatArg) {}
win32_error::win32_error(const char* whatArg): std::system_error(getLastError(), whatArg) {}

////////////////////////////////////////////////////////////////////////////////
// Win32 style stacktrace
////////////////////////////////////////////////////////////////////////////////

#if (defined(__clang__) && defined(DEBUG_CLANG_USE_PDB)) || defined(_MSC_VER)
// Windows can only read symbols from PDBs, which only Clang and MSVC are able to generate.
// For other compilers, the stacktrace is meaningless without symbols.

#define SUPPORTS_STACKTRACE

namespace {

class SymbolGuard {
public:
    SymbolGuard(HANDLE process, PCWSTR searchPath):
            process_(process), didLoad_(SymInitializeW(process, searchPath, true)) {}
    SymbolGuard(const SymbolGuard&) = delete;
    SymbolGuard(SymbolGuard&&) = delete;
    SymbolGuard& operator=(const SymbolGuard&) = delete;
    SymbolGuard& operator=(SymbolGuard&&) = delete;
    ~SymbolGuard() {
        if (didLoad_) {
            SymCleanup(process_);
        }
    }

private:
    HANDLE process_;
    bool didLoad_;
};

template <class NextF>
void printStacktrace(std::ostream& stream, HANDLE process, HANDLE thread, unsigned short maxFrames,
                     NextF& nextAddress) {
    static_assert(std::is_invocable_r_v<bool, NextF, unsigned short, uint64_t&>);

#pragma region local struct decorated symbol info
    constexpr size_t symbolNameLength = 200;

    /**
     * You're supposed to allocate more memory for the symbol name at the end of the SYMBOL_INFO object.
     * This struct provides this plus appropriate initialization of SizeOfStruct and MaxNameLen members.
     */
    struct MySymbolInfo {
        MySymbolInfo() {
            symbolInfo_.SizeOfStruct = sizeof(SYMBOL_INFOW);
            symbolInfo_.MaxNameLen = strict_cast<unsigned long>(symbolName_.size());
        }

        SYMBOL_INFOW& symbolInfo() { return symbolInfo_; }
        std::wstring_view symbolName() {
            return static_cast<wchar_t*>(symbolInfo_.Name);  // Symbol name goes past the end of the array
                                                             // => don't use fixed-length char array constructor
        }

    private:
        SYMBOL_INFOW symbolInfo_ = {};
        [[maybe_unused]] std::array<wchar_t, symbolNameLength> symbolName_ = {};
    };

    static_assert(std::is_standard_layout_v<MySymbolInfo>);  // Safety to access non-static data members via pointer.
    static_assert(sizeof(MySymbolInfo) ==
                  sizeof(SYMBOL_INFOW) + symbolNameLength * sizeof(wchar_t));  // Ensure no padding.
#pragma endregion

    SymbolGuard symbolGuard(process, nullptr);  // load symbols

    for (unsigned short frameIndex = 0, unknownSkipped = 0; frameIndex < maxFrames; frameIndex++) {
        uint64_t address = 0;
        if (!nextAddress(frameIndex, address))
            break;

        // get symbol name for address
        unsigned long long symDisplacement = 0;
        MySymbolInfo symbol;
        if (!SymFromAddrW(process, address, &symDisplacement, &symbol.symbolInfo())) {
            unknownSkipped++;
            continue;
        } else if (unknownSkipped) {
            stream << "    (skipped " << unknownSkipped << " frames with unknown symbol)" << std::endl;
            unknownSkipped = 0;
        }

        // try to get line
        IMAGEHLP_LINEW64 lineInfo = {};
        lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);
        unsigned long wLineDisplacement = 0;
        if (SymGetLineFromAddrW64(process, address, &wLineDisplacement, &lineInfo)) {
            stream << "    at " << StringCvt::u8(lineInfo.FileName) << ':' << lineInfo.LineNumber << '\n'
                   << "        (address 0x" << reinterpret_cast<void*>(address) << " in "
                   << StringCvt::u8(symbol.symbolName()) << ')';
        } else {
            // failed to get line
            stream << "    at address 0x" << reinterpret_cast<void*>(address) << " in "
                   << StringCvt::u8(symbol.symbolName());

            // at least print module name
            IMAGEHLP_MODULEW64 moduleInfo = {};
            moduleInfo.SizeOfStruct = sizeof(moduleInfo);
            if (SymGetModuleInfoW64(process, address, &moduleInfo)) {
                auto moduleName = fs::path(moduleInfo.ImageName).filename();
                stream << " in " << moduleName.u8string();
            }
        }
        stream << std::endl;
    }
}

}  // namespace

#else  // (__clang__ && DEBUG_CLANG_USE_PDB) || _MSC_VER

void printStacktraceUnsupported(std::ostream& stream) {
    constexpr const char* compiler =
#ifdef __GNUG__
            "GCC";
#else
            "(unknown compiler)";
#endif

    stream << "    Stacktrace is not supported when building with " << compiler << std::endl;
}

#endif  // (__clang__ && DEBUG_CLANG_USE_PDB) || _MSC_VER

__declspec(noinline) void printStacktrace(std::ostream& stream, HANDLE process, HANDLE thread,
                                          unsigned short skipFrames, unsigned short maxFrames) {

#ifdef SUPPORTS_STACKTRACE
    std::vector<void*> backtrace(maxFrames, 0);
    auto frameCount = CaptureStackBackTrace(skipFrames + 1, maxFrames, backtrace.data(), nullptr);
    auto nextAddress = [&](unsigned short frameIndex, uint64_t& address) {
        if (address < backtrace.size()) {
            address = reinterpret_cast<uintptr_t>(backtrace.at(frameIndex));
            return true;
        }
        return false;
    };
    printStacktrace(stream, process, thread, frameCount, nextAddress);
#else   // SUPPORTS_STACKTRACE
    printStacktraceUnsupported(stream);
#endif  // SUPPORTS_STACKTRACE
}

__declspec(noinline) void printStacktrace(std::ostream& stream, unsigned short skipFrames, unsigned short maxFrames) {
#ifdef SUPPORTS_STACKTRACE
    printStacktrace(stream, GetCurrentProcess(), GetCurrentThread(), skipFrames + 1, maxFrames);
#else
    printStacktraceUnsupported(stream);
#endif
}

void printStacktrace(std::ostream& stream, HANDLE process, HANDLE thread, unsigned short maxFrames, CONTEXT* context) {
#ifdef SUPPORTS_STACKTRACE
    STACKFRAME64 stackFrame = {};

#if defined(_M_AMD64)
    stackFrame.AddrPC.Offset = context->Rip;
    stackFrame.AddrFrame.Offset = context->Rbp;
    stackFrame.AddrStack.Offset = context->Rsp;
    stackFrame.AddrPC.Mode = stackFrame.AddrFrame.Mode = stackFrame.AddrStack.Mode = AddrModeFlat;
#elif defined(_M_IX86)
    stackFrame.AddrPC.Offset = context->Eip;
    stackFrame.AddrFrame.Offset = context->Ebp;
    stackFrame.AddrStack.Offset = context->Esp;
    stackFrame.AddrPC.Mode = stackFrame.AddrFrame.Mode = stackFrame.AddrStack.Mode = AddrModeFlat;
#else
#error Unknown processor architecture?
#endif

    auto nextAddress = [&](unsigned short, uint64_t& address) {
        const bool r = StackWalk64(
#if defined(_M_AMD64)
                IMAGE_FILE_MACHINE_AMD64
#elif defined(_M_IX86)
                IMAGE_FILE_MACHINE_I386
#else
#error Unknown processor architecture?
#endif
                ,
                process, thread, &stackFrame, context, nullptr, &SymFunctionTableAccess64, &SymGetModuleBase64,
                nullptr);
        address = stackFrame.AddrPC.Offset;
        return r;
    };
    printStacktrace(stream, process, thread, maxFrames, nextAddress);
#else   // SUPPORTS_STACKTRACE
    printStacktraceUnsupported(stream);
#endif  // SUPPORTS_STACKTRACE
}

////////////////////////////////////////////////////////////////////////////////
// Misc utils
////////////////////////////////////////////////////////////////////////////////

auto getModuleFileName(HMODULE hModule) -> fs::path {
    std::wstring moduleFileName(MAX_PATH, '\0');

    // GetModuleFileName returns number of chars excluding trailing \0 if path
    // fits into buffer including trailing \0, else returns buffer size.
    unsigned long charCount = 0;
    while ((charCount = GetModuleFileNameW(hModule, moduleFileName.data(),
                                           strict_cast<unsigned long>(moduleFileName.size()))) ==
           moduleFileName.size()) {
        moduleFileName.resize(2 * moduleFileName.size(), '\0');
    }
    if (!charCount) {
        throw win32_error("GetModuleFileName failed");
    }
    moduleFileName.resize(charCount);
    return fs::path(std::move(moduleFileName));
}

namespace detail {

static constexpr unsigned char waitResultFlagTimeout = 0b0001;
static constexpr unsigned char waitResultFlagSignaled = 0b0010;
static constexpr unsigned char waitResultFlagAbandoned = 0b0110;

auto waitForObjects(unsigned long count, const HANDLE* objects, bool all, WaitTimeout timeout) -> WaitResult {
    static_assert(WAIT_OBJECT_0 == 0);
    assert(count > 0);

    WaitResult result;

    unsigned long timeoutMs = std::holds_alternative<infinite_t>(timeout) ? INFINITE : std::get<unsigned long>(timeout);

    if (const auto r = WaitForMultipleObjects(count, objects, all, timeoutMs); r < WAIT_OBJECT_0 + count) {
        result.flags_ = waitResultFlagSignaled;
        result.index_ = r;
    } else {
        switch (r) {
            case WAIT_TIMEOUT:
                result.flags_ = waitResultFlagTimeout;
                break;
            case WAIT_FAILED:
                throw win32_error("WaitForMultipleObjects failed");
                break;
            default:
                result.flags_ = waitResultFlagSignaled | waitResultFlagAbandoned;
                result.index_ = r - WAIT_ABANDONED_0;
        }
    }

    return result;
}

}  // namespace detail

bool WaitResult::done() const {
    assert(flags_);
    return flags_ & detail::waitResultFlagSignaled;
}
bool WaitResult::isAbandoned() const { return flags_ & detail::waitResultFlagAbandoned; }
unsigned long WaitResult::index() const {
    assert(done());
    return index_;
}

}  // namespace xoj::win32

#endif  // _WIN32
