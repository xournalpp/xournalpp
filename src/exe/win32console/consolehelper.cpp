// FIXME Guard idea from GPL?

#include <sstream>

#include <glib.h>
#include <windows.h>

#include "util/PathUtil.h"
#include "util/Win32Util.h"
#include "util/logger/Dumplog.h"

#include "config-debug.h"

#define ch_message(...) g_message("[Console Helper] " __VA_ARGS__)    // NOLINT
#define ch_warning(...) g_warning("[Console Helper] " __VA_ARGS__)    // NOLINT
#define ch_critical(...) g_critical("[Console Helper] " __VA_ARGS__)  // NOLINT

#ifdef DEBUG_CONSOLE
#define ch_dbg_message(...) ch_message(__VA_ARGS__)  // NOLINT
#else
#define ch_dbg_message(...)
#endif

static const HWND consoleWindow = GetConsoleWindow();
static const HMENU windowMenu = GetSystemMenu(consoleWindow, false);
static const HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);

struct NumbConsole {  // NOLINT
    NumbConsole() {
        EnableMenuItem(windowMenu, SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);
        SetConsoleMode(inputHandle, 0);
    }

    ~NumbConsole() {
        EnableMenuItem(windowMenu, SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
        SetConsoleMode(inputHandle, ENABLE_PROCESSED_INPUT);
    }
};

enum class Argv : size_t { _0, xppProcess, xppMainThread, readyEvent, heartbeatEvent, numArgs };

template <class E>
constexpr auto ut(E v) {
    return std::underlying_type_t<E>(v);
}

int main(int argc, char* argv[]) {
    ch_dbg_message("Started console helper");

    using Error = struct {};
    try {
        NumbConsole numbConsole;
#ifdef DEBUG_CONSOLE
        if (IsWindowVisible(consoleWindow)) {
            ch_warning("You can now attach a debugger to the console helper process.\n"
                       "Press any key to continue or press <x> to kill the console helper process.\n");
            wchar_t c = 0;
            unsigned long nc = 0;
            if (ReadConsoleW(inputHandle, &c, 1, &nc, nullptr) && c == 'x') {
                return 1;
            }
        }
#endif

        if (argc < ut(Argv::numArgs)) {
            ch_critical("Not enough handles passed");
            throw Error();
        }

        {
            xoj::win32::UniqueHandle readyEvent;
            std::istringstream sReadyEvent(argv[ut(Argv::readyEvent)]);
            sReadyEvent >> readyEvent.oref();
            if (readyEvent) {
                SetEvent(readyEvent.get());
            }
        }

        xoj::win32::UniqueHandle xppProcess, xppMainThread, heartbeatEvent;
        {
            std::istringstream sXppProcess(argv[ut(Argv::xppProcess)]);
            xoj::win32::UniqueHandle inheritedXppProcess;
            sXppProcess >> inheritedXppProcess.oref();
            const auto cp = GetCurrentProcess();
            if (!DuplicateHandle(cp, inheritedXppProcess.get(), cp, xppProcess.oref(),
                                 SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, 0)) {
                ch_critical("Duplicating Xournal++ process handle failed: %s",
                            xoj::win32::getLastError().message().c_str());
            }

            std::istringstream sXppMainThread(argv[ut(Argv::xppMainThread)]);
            xoj::win32::UniqueHandle inheritedXppMainThread;
            sXppMainThread >> inheritedXppMainThread.oref();
            if (!DuplicateHandle(cp, inheritedXppMainThread.get(), cp, xppMainThread.oref(),
                                 THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, false, 0)) {
                ch_critical("Duplicating Xournal++ main thread handle failed: %s",
                            xoj::win32::getLastError().message().c_str());
            }

            std::istringstream sHeartbeatEvent(argv[ut(Argv::heartbeatEvent)]);
            sHeartbeatEvent >> heartbeatEvent.oref();
        }

        if (!(xppProcess && xppMainThread && heartbeatEvent)) {
            ch_critical("Invalid handles passed:"
                        "\n    xppProcess = %p"
                        "\n    xppMainThreadProcess = %p"
                        "\n    heartbeatEvent = %p",
                        xppProcess.get(), xppMainThread.get(), heartbeatEvent.get());
            throw Error();
        }

        constexpr unsigned long timeoutSec = 7;

        try {
            if (const auto r = xoj::win32::waitForMultipleObjects({xppProcess.get(), heartbeatEvent.get()},
                                                                  false, timeoutSec * 1000)) {
                switch (r.index()) {
                    case 1:
                        ch_dbg_message("Received heartbeat");
                        break;
                    default:
                        break;
                }
            } else {
                // timeout
                using namespace xoj::util::log;
                Dumplog dumplog(Dumplog::DEADLOG, "[Console Helper] ", Dumplog::HEADER_TIME);
                dumplog.addHeaderLine("Potential deadlock");

                if (SuspendThread(xppMainThread.get()) == -1) {
                    dumplog << "Couldn't suspend main thread to get a stacktrace: "
                            << xoj::win32::getLastError().message();
                } else {
                    CONTEXT context = {};
                    context.ContextFlags = CONTEXT_ALL;
                    if (!GetThreadContext(xppMainThread.get(), &context)) {
                        dumplog << "Couldn't get thread context: " << xoj::win32::getLastError().message();
                    } else {
                        dumplog << "Main thread stacktrace:\n";
                        xoj::win32::printStacktrace(dumplog.stream(), xppProcess.get(), xppMainThread.get(), 256,
                                                    &context);
                    }
                    ResumeThread(xppMainThread.get());
                }

                std::ostringstream s;
                s << "Didn't receive a heartbeat in " << timeoutSec << " seconds.\n"
                    << "Xournal++ might have deadlocked. " << printBugreportNotice << '\n'
                    << dumplog.str();
                ch_warning("%s", s.str().c_str());

                throw Error();
            }

            if (xoj::win32::waitForSingleObject(xppProcess.get(), xoj::win32::infinite)) {
#ifdef DEBUG_CONSOLE
                if (unsigned long exitCode = 0; GetExitCodeProcess(xppProcess.get(), &exitCode)) {
                    ch_message("Xournal++ exited with code %lu", exitCode);
                } else {
                    ch_warning("Xournal++ exited. Failed to get exit code:\n"
                               "    %s",
                               xoj::win32::getLastError().message().c_str());
                }
#endif
            }

        } catch (xoj::win32::win32_error& e) {
            ch_critical("Waiting for process exit or heartbeat failed: %s", e.code().message().c_str());
            throw Error();
        }

    } catch (Error&) {
        ch_warning("Closing the console window will kill Xournal++.\n");

        // The window might not show on the first call to ShowWindow if startupInfo uses show
        // command with SW_HIDE
        ShowWindow(consoleWindow, SW_SHOWDEFAULT);
        ShowWindow(consoleWindow, IsIconic(consoleWindow) ? SW_RESTORE : SW_SHOW);
        SetForegroundWindow(consoleWindow);
    }

    wchar_t c = 0;
    unsigned long nc = 0;
    while (ReadConsoleW(inputHandle, &c, 1, &nc, nullptr)) {}

    return 0;
}