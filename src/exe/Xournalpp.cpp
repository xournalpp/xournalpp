/*
 * Xournal++
 *
 * The main application
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#include <string_view>

#include "control/Console.h"
#include "control/CrashHandler.h"
#include "control/XournalMain.h"
#include "util/logger/Logger.h"

#include "filesystem.h"

auto main(int argc, char* argv[]) -> int {
    bool showConsole = false;
    if (argc >= 2) {
        using namespace std::string_view_literals;

        for (char **argPtr = argv + 1, **end = argv + argc; argPtr != end; argPtr++) {
            if (*argPtr == "--"sv) {
                break;
            }
            if (*argPtr == "--show-console"sv) {
                showConsole = true;
                continue;
            }
        }
    }

    // Attach to the console here. Otherwise, gspawn-win32-helper will create annoying console popups.
    ConsoleCtl::initPlatformConsole(showConsole);

    // init crash handler
    installCrashHandlers();

#ifdef DEV_CALL_LOG
    Log::initlog();
#endif

#ifdef _WIN32
    // Switch to the FontConfig backend for Pango - See #3371
    _putenv_s("PANGOCAIRO_BACKEND", "fc");
#endif

    // Use this two line to test the crash handler...
    // int* crash = nullptr;
    // *crash = 0;

    int result = XournalMain::run(argc, argv);

#ifdef DEV_CALL_LOG
    Log::closelog();
#endif

    return result;
}
