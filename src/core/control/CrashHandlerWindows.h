/*
 * Xournal++
 *
 * This is the Windows Crash Handler implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <utility>

#include <gtk/gtk.h>
#include <windows.h>

#include "control/Console.h"
#include "util/PathUtil.h"
#include "util/StringUtils.h"
#include "util/Win32Util.h"
#include "util/i18n.h"
#include "util/logger/Dumplog.h"

#include "CrashHandler.h"
#include "config-dev.h"


static constexpr const char* crashHandlerPrefix = "[Crash Handler] ";

static bool alreadyCrashed_ = false;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static LONG WINAPI win32SehCrashHandler(LPEXCEPTION_POINTERS ex);
static void win32CppCrashHandler();

void installCrashHandlers() {
    SetUnhandledExceptionFilter(&win32SehCrashHandler);
    std::set_terminate(win32CppCrashHandler);
}

constexpr std::array ect = {std::pair{EXCEPTION_ACCESS_VIOLATION, "Access violation"},
                            std::pair{EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "Array bounds exceeded"},
                            std::pair{EXCEPTION_BREAKPOINT, "Breakpoint"},
                            std::pair{EXCEPTION_DATATYPE_MISALIGNMENT, "Datatype misalignment"},
                            std::pair{EXCEPTION_FLT_DENORMAL_OPERAND, "Flt denormal operand"},
                            std::pair{EXCEPTION_FLT_DIVIDE_BY_ZERO, "Flt divide by zero"},
                            std::pair{EXCEPTION_FLT_INEXACT_RESULT, "Flt inexact result"},
                            std::pair{EXCEPTION_FLT_INVALID_OPERATION, "Flt invalid operation"},
                            std::pair{EXCEPTION_FLT_OVERFLOW, "Flt overflow"},
                            std::pair{EXCEPTION_FLT_STACK_CHECK, "Flt stack check"},
                            std::pair{EXCEPTION_FLT_UNDERFLOW, "Flt underflow"},
                            std::pair{EXCEPTION_ILLEGAL_INSTRUCTION, "Illegal instruction"},
                            std::pair{EXCEPTION_IN_PAGE_ERROR, "In page error"},
                            std::pair{EXCEPTION_INT_DIVIDE_BY_ZERO, "Int divide by zero"},
                            std::pair{EXCEPTION_INT_OVERFLOW, "Int overflow"},
                            std::pair{EXCEPTION_INVALID_DISPOSITION, "Invalid disposition"},
                            std::pair{EXCEPTION_NONCONTINUABLE_EXCEPTION, "Noncontinuable exception"},
                            std::pair{EXCEPTION_PRIV_INSTRUCTION, "Priv instruction"},
                            std::pair{EXCEPTION_SINGLE_STEP, "Single step"},
                            std::pair{EXCEPTION_STACK_OVERFLOW, "Stack overflow"}};

static const char* translateExceptionCode(unsigned long ec) {
    const auto it = std::find_if(ect.begin(), ect.end(), [&ec](auto& it) { return it.first == ec; });
    return it == ect.end() ? nullptr : it->second;
}

static void printExceptionRecord(std::ostream& stream, EXCEPTION_RECORD& er, unsigned int depth = 0) {
    std::string prefix(depth * 2, ' ');
    if (auto translated = translateExceptionCode(er.ExceptionCode)) {
        stream << translated;
    } else {
        stream << prefix << "Code: " << er.ExceptionCode;
    }
    stream << "\n";

    if (er.ExceptionRecord) {
        stream << "Associated:\n";
        printExceptionRecord(stream, *er.ExceptionRecord, depth + 1);
    }
}

class CrashHandlerCommon {  // NOLINT(cppcoreguidelines-special-member-functions)
public:
    using Dumplog = xoj::util::log::Dumplog;

    CrashHandlerCommon(const char* reason, Dumplog&& d): dumplog_(std::move(d)) {
        if (alreadyCrashed_) {
            exit(2);
        }
        alreadyCrashed_ = true;

        g_critical("%sCrashed due to %s", crashHandlerPrefix, reason);

        dumplog_->addHeaderLine(std::string("Error: ").append(reason));
    }

    ~CrashHandlerCommon() {
        g_message("%s%s", crashHandlerPrefix, StringUtils::trim(dumplog_->str()).c_str());

        emergencySave();

        if (auto platformConsole = ConsoleCtl::getPlatformConsole().value_or(nullptr)) {
            platformConsole->setShowOnDestruction(true);
            platformConsole->show(true);
        }

        dumplog_.reset();  // need to destroy Dumplog object before detaching from console
        ConsoleCtl::abnormalExit();

        g_message("The console can be closed.");
    }

    auto dumplog() -> Dumplog& { return *dumplog_; }

private:
    std::optional<Dumplog> dumplog_;
};


LONG WINAPI win32SehCrashHandler(LPEXCEPTION_POINTERS ex) {
    {
        using namespace xoj::util::log;

        constexpr const char* reason = "SEH exception";
        CrashHandlerCommon chCommon(reason, Dumplog(Dumplog::ERRORLOG, crashHandlerPrefix, Dumplog::HEADER_TIME));

        printExceptionRecord(chCommon.dumplog().stream(), *ex->ExceptionRecord);
        xoj::win32::printStacktrace(chCommon.dumplog().stream(), GetCurrentProcess(), GetCurrentThread(), 50,
                                    ex->ContextRecord);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

void win32CppCrashHandler() {
    {
        using namespace xoj::util::log;

        constexpr const char* reason = "C++ exception";
        CrashHandlerCommon chCommon(reason, Dumplog(Dumplog::ERRORLOG, crashHandlerPrefix, Dumplog::HEADER_TIME));

        auto exPtr = std::current_exception();
        if (exPtr) {
            try {
                std::rethrow_exception(exPtr);
            } catch (std::exception& e) { chCommon.dumplog() << e.what() << '\n'; }
        } else {
            chCommon.dumplog() << "No exception caught.\n";
        }

#ifdef __clang__
        constexpr auto skipFrames = 4;
#else
        constexpr auto skipFrames = 1;
#endif
        xoj::win32::printStacktrace(chCommon.dumplog().stream(), skipFrames, 50);
    }
    std::abort();
}
