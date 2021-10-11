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
#include "util/logger/Logger.h"

#include "CrashHandler.h"
#include "Stacktrace.h"
#include "filesystem.h"

#ifdef _WIN32
#include "win32/console.h"
#endif

auto main(int argc, char* argv[]) -> int {
#ifdef _WIN32
    // Attach to the console here. Otherwise, gspawn-win32-helper will create annoying console popups.
    attachConsole();
#endif

    /*
     * Set the current working directory to the application directory.
     * Otherwise, translations are handled inconsistently.
     * GitHub #3433
     */
    std::string exePath = std::string(argv[0]);
    std::string::size_type pos = exePath.find_last_of("\\/");
    fs::current_path(exePath.substr(0, pos));

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
