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

#include <config-dev.h>

#include "control/XournalMain.h"

#include "CrashHandler.h"
#include "Stacktrace.h"

#ifdef _WIN32
#include "win32/console.h"
#endif

auto main(int argc, char* argv[]) -> int {
#ifdef _WIN32
    // Attach to the console here. Otherwise, gspawn-win32-helper will create annoying console popups.
    attachConsole();
#endif

    // init crash handler
    installCrashHandlers();

#ifdef DEV_CALL_LOG
    Log::initlog();
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
