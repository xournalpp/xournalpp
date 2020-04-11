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

#include "control/XournalMain.h"

#include <config-dev.h>
#include <CrashHandler.h>
#include <Stacktrace.h>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // Show and hide the console here. Otherwise, gspawn-win32-helper will create annoying console popups.
    AllocConsole();
    ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif

	// init crash handler
	installCrashHandlers();

#ifdef DEV_CALL_LOG
	Log::initlog();
#endif

	// Use this two line to test the crash handler...
//	int* crash = NULL;
//	*crash = 0;

	XournalMain* main = new XournalMain();
	int result = main->run(argc, argv);
	delete main;

#ifdef DEV_MEMORY_LEAK_CHECKING
	xoj_momoryleak_printRemainingObjects();
#endif

#ifdef DEV_CALL_LOG
	Log::closelog();
#endif

	return result;
}
