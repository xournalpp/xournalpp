// FIXME Guard idea from GPL (if @jaburjak agrees?)

#ifndef _WIN32
#error This is intended for Windows only
#endif

#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <tuple>

#include <io.h>
#include <windows.h>

#include "util/StringUtils.h"
#include "util/Win32Util.h"
#include "util/logger/Dumplog.h"

#include "Console.h"
#include "config-debug.h"


namespace {

constexpr const std::wstring_view consoleHelperExecutable{L"console-helper.exe"};

const bool hadStdout = GetStdHandle(STD_OUTPUT_HANDLE);
const bool hadStderr = GetStdHandle(STD_ERROR_HANDLE);
const bool isDebuggerPresent = IsDebuggerPresent();
const bool noConsole = !isDebuggerPresent && (hadStdout && hadStderr);

struct ConsoleHelperError {
private:
    std::string stage_;
    std::error_code ec_;

public:
    void set(const char* stage) { set(stage, xoj::win32::win32_errc{0}); }
    void set(const char* stage, const std::error_code& ec) {
        if (!*this) {
            stage_ = stage;
            ec_ = ec;
        }
    }

    explicit operator bool() const { return !stage_.empty() || ec_; }
    std::string str() const {
        std::ostringstream ostream;
        ostream << stage_.c_str();
        if (ec_) {
            ostream << ": " << ec_.message();
        }
        return ostream.str();
    }
};

std::wstring getConsoleHelperCl(HANDLE currentProcess, HANDLE currentThread, HANDLE readyEvent, HANDLE heartbeatEvent) {
    const fs::path consoleHelperPath{xoj::win32::getModuleFileName(nullptr).remove_filename() /=
                                     consoleHelperExecutable};

    std::wstringstream consoleHelperClStream;
    consoleHelperClStream << '"' << consoleHelperPath.native() << "\" " << currentProcess << ' ' << currentThread << ' '
                          << readyEvent << ' ' << heartbeatEvent;

    return consoleHelperClStream.str();
}

}  // namespace

class Win32Console final: public PlatformConsole {
public:
    explicit Win32Console(bool show) {
        if (GetConsoleWindow()) {
            // Console is already attached.
            return;
        }

        const bool effectiveShow = noConsole ? false : show;
        this->showOnDestruction_ = !noConsole;
        ConsoleHelperError consoleHelperError;
#ifndef NDEBUG
        if (!effectiveShow && isDebuggerPresent && AttachConsole(ATTACH_PARENT_PROCESS)) {
        } else
#endif  // !NDEBUG
        {
            myAllocConsole(effectiveShow, consoleHelperError);
        }

        auto reopen = [](int fd, FILE* stdFile, const char* conFileName, std::basic_ios<char>& stream,
                         std::basic_ios<wchar_t>& wstream, const char* streamDesc) {
            // Close file and file descriptor manually, since if stdFile is not
            // open, its file descriptor won't be closed and reused by freopen_s
            fclose(stdFile);
            _close(fd);

            FILE* dummyFile = nullptr;
            if (const auto r = freopen_s(&dummyFile, conFileName, "r+", stdFile)) {  // Get/SetConsoleMode (e.g. for
                                                                                     // colored output) requires read
                                                                                     // access
                // For debugging
                std::ostringstream s;
                s << "Failed to reopen " << streamDesc << ": " << std::generic_category().message(r).c_str();
                OutputDebugStringW(StringCvt::u16(s.str()).c_str());

                // Continuing with potentially closed stdout/stderr will lead to UB
                //  => crash the process
                std::abort();
            }
            if (const auto r = setvbuf(stdFile, nullptr, _IONBF, 0)) {  // _IOLBF would be equivalent to _IOFBF on Win32
                std::ostringstream s;
                s << "Failed to disable buffering for " << streamDesc << ": "
                  << std::generic_category().message(r).c_str();
                OutputDebugStringW(StringCvt::u16(s.str()).c_str());
            }
            stream.clear();
            wstream.clear();

            if (_fileno(stdFile) != fd) {
                std::ostringstream s;
                s << "Reopened " << streamDesc << ": Couldn't reuse file descriptor.";
                OutputDebugStringW(StringCvt::u16(s.str()).c_str());
            }
        };

        if (!hadStdout || (isDebuggerPresent && effectiveShow)) {
            reopen(STDOUT_FILENO, stdout, "CONOUT$", std::cout, std::wcout, "standard output stream");
        }
        if (!hadStderr || (isDebuggerPresent && effectiveShow)) {
            reopen(STDERR_FILENO, stderr, "CONOUT$", std::cerr, std::wcerr, "standard error stream");
        }

        if ((hadStdout || hadStderr) && !(isDebuggerPresent && effectiveShow)) {
            std::ofstream tmpOut("CONOUT$", std::ios_base::out);
            if (hadStdout) {
                tmpOut << "[stdout has been redirected]" << std::endl;
            }
            if (hadStderr) {
                tmpOut << "[stderr has been redirected]" << std::endl;
            }
        }

        if (consoleHelperError) {
            std::cerr << "An error occured while spawning Xournal++ console:\n"
                      << "  " << consoleHelperError.str() << '\n'
                      << xoj::util::log::printBugreportNotice << "\n\n"
                      << std::flush;
        }
    }

    Win32Console(const Win32Console&) = delete;
    Win32Console(Win32Console&&) = delete;
    Win32Console& operator=(const Win32Console&) = delete;
    Win32Console& operator=(Win32Console&&) = delete;
    ~Win32Console() {
        if (this->showOnDestruction_) {
            detachConsoleHelper();
            show(false);
        }
        if (this->hasOwnConsoleWindow_) {
            FreeConsole();
        }
    }

    void show(bool flash) override {
        if (noConsole)
            return;

        showUnconditionally(flash);
    }

    void showUnconditionally(bool flash) override {
        if (!this->hasOwnConsoleWindow_)
            return;

        HWND consoleWindow = GetConsoleWindow();
        ShowWindow(consoleWindow, IsIconic(consoleWindow) ? SW_RESTORE : SW_SHOW);
        SetForegroundWindow(consoleWindow);

        if (flash) {
            FLASHWINFO flashWInfo = {};
            flashWInfo.cbSize = sizeof(flashWInfo);
            flashWInfo.hwnd = GetConsoleWindow();
            // Flash window until it gets focus
            flashWInfo.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
            FlashWindowEx(&flashWInfo);
        }
    }

    void hide() override {
        if (!this->hasOwnConsoleWindow_)
            return;

        ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

    void sendHeartbeat() override {
        if (this->heartbeatEvent_)
            SetEvent(this->heartbeatEvent_.get());
    }

    bool hasOwnConsoleWindow() const { return this->hasOwnConsoleWindow_; }

    void setShowOnDestruction(bool show) override {
        if (noConsole)
            return;

        this->showOnDestruction_ = show;
    }

private:
    bool hasOwnConsoleWindow_ = false;
    xoj::win32::UniqueHandle consoleHelperJob_;
    xoj::win32::UniqueHandle heartbeatEvent_;
    bool showOnDestruction_ = !noConsole;

    void myAllocConsole(bool effectiveShow, ConsoleHelperError& consoleHelperError) {
        SECURITY_ATTRIBUTES saInheritableHandle = {};
        saInheritableHandle.nLength = sizeof(saInheritableHandle);
        saInheritableHandle.bInheritHandle = true;

        using ConsoleAllocError = std::tuple<const char*, std::optional<xoj::win32::win32_errc>>;
        struct ProcessTerminator {
            void operator()(HANDLE hProcess) {
                if (!TerminateProcess(hProcess, 0)) {
                    throw xoj::win32::win32_error("TerminateProcess failed");
                }
            }
        };
        using UniqueProcess = std::unique_ptr<void, ProcessTerminator>;
        try {
            // FIXME More secure solution than passing around process and thread handles?
            xoj::win32::UniqueHandle currentProcess, currentThread;
            {
                // GetCurrentProcess/Thread() return a pseudo handle that is interpreted as handle to current
                // process/thread. Need real process handle to pass to console helper.
                // Don't need any access rights on these handles since console helper will duplicate them with needed
                // access rights.
                HANDLE cp = GetCurrentProcess();
                if (!DuplicateHandle(cp, cp, cp, currentProcess.oref(), 0, true, 0)) {
                    throw ConsoleAllocError{"Duplicating current process handle", xoj::win32::getLastError()};
                }
                if (!DuplicateHandle(cp, GetCurrentThread(), cp, currentThread.oref(), 0, true, 0)) {
                    throw ConsoleAllocError{"Duplicating current thread handle", xoj::win32::getLastError()};
                }
            }

            // Event to signal when console helper is ready to attach
            xoj::win32::UniqueHandle readyEvent = CreateEventW(&saInheritableHandle, true, false, nullptr);
            if (!readyEvent) {
                throw ConsoleAllocError{"Init'ing readyEvent", xoj::win32::getLastError()};
            }

            xoj::win32::UniqueHandle heartbeatEvent = CreateEventW(&saInheritableHandle, false, false, nullptr);
            if (!heartbeatEvent) {
                throw ConsoleAllocError{"Init'ing heartbeatEvent", xoj::win32::getLastError()};
            }

            // Make sure the console starts hidden if requested.
            STARTUPINFOW startupInfo = {};
            startupInfo.cb = sizeof(startupInfo);
            if (!effectiveShow) {
                startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
                startupInfo.wShowWindow = SW_HIDE;
            }
            PROCESS_INFORMATION processInformation = {};
            if (!CreateProcessW(nullptr,
                                getConsoleHelperCl(currentProcess.get(), currentThread.get(), readyEvent.get(),
                                                   heartbeatEvent.get())
                                        .data(),
                                nullptr, nullptr, true, 0, nullptr, nullptr, &startupInfo, &processInformation)) {
                throw ConsoleAllocError{"Spawning console process", xoj::win32::getLastError()};
            }

            const xoj::win32::UniqueHandle process{processInformation.hProcess};
            const xoj::win32::UniqueHandle thread{processInformation.hThread};
            UniqueProcess processGuard{process.get()};  // terminate console helper process when leaving scope unless
                                                        // released

            xoj::win32::UniqueHandle job = CreateJobObjectW(nullptr, nullptr);
            if (!job) {
                throw ConsoleAllocError{"Init'ing console helper job", xoj::win32::getLastError()};
            }

            // Terminate the console process automatically when Xournal++ exits.
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInformation = {};
            jobInformation.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

            if (!SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &jobInformation,
                                         sizeof(jobInformation)) ||
                !AssignProcessToJobObject(job.get(), processInformation.hProcess)) {

                throw ConsoleAllocError{"Assigning console helper job", xoj::win32::getLastError()};
            }

            processGuard.release();  // console helper process is now controlled by job object

#ifdef DEBUG_CONSOLE
            const unsigned long timeoutSec = effectiveShow ? 20 : 2;
#else
            constexpr unsigned long timeoutSec = 2;
#endif
            if (auto r =
                        xoj::win32::waitForMultipleObjects({readyEvent.get(), process.get()}, false, timeoutSec * 1000);
                r.index() == 1) {

                throw ConsoleAllocError{"Console process terminated", {}};
            } else if (!r) {
                throw ConsoleAllocError{"(timeout) Waiting for console process", {}};
            }
            // readyEvent is not a mutex so no need to check if abandoned

            if (!AttachConsole(processInformation.dwProcessId)) {
                throw ConsoleAllocError{"Attaching console", xoj::win32::getLastError()};
            }

            this->consoleHelperJob_ = std::move(job);
            this->heartbeatEvent_ = std::move(heartbeatEvent);

        } catch (ConsoleAllocError& e) {
            const auto& [stage, optEc] = e;
            if (optEc) {
                consoleHelperError.set(stage, optEc.value());
            } else {
                consoleHelperError.set(stage);
            }

            // Could not attach to the manually created console process, request one from the system instead.
            // [suspended ->] This will make the console window flash briefly before we hide it. [<-]
            AllocConsole();

            // Crash handler is useless if the console window disappears right after Xournal++ process
            // terminates. Don't hide console window so users will report issues with attaching console helper.
            // ShowWindow(GetConsoleWindow(), SW_HIDE);
        }

        this->hasOwnConsoleWindow_ = true;

        SetConsoleTitleW(L"Xournal++ Console");
        SetConsoleOutputCP(CP_UTF8);
    }

    /**
     * If console helper is running, decouples its lifetime from Xournal++ process lifetime.
     * E.g. useful in crash handler.
     */
    void detachConsoleHelper() {
        if (!this->consoleHelperJob_)
            return;

        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInformation = {};
        SetInformationJobObject(this->consoleHelperJob_.get(), JobObjectExtendedLimitInformation, &jobInformation,
                                sizeof(jobInformation));
    }
};

namespace {

std::optional<Win32Console> globalConsole;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
                                            // Static object so that destructor is called on exit

}  // namespace

void ConsoleCtl::initPlatformConsole(bool show) {
    if (globalConsole)
        throw std::logic_error("Console already initialized.");

    globalConsole.emplace(show);
}

auto ConsoleCtl::getPlatformConsole() -> std::optional<PlatformConsole*> {
    std::optional<PlatformConsole*> ret;
    if (globalConsole)
        ret = &*globalConsole;

    return ret;
}

void ConsoleCtl::abnormalExit() { globalConsole.reset(); }
