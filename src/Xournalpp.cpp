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

int main(int argc, char* argv[])
{
	// init crash handler
	installCrashHandlers();

#ifdef DEV_CALL_LOG
	Log::initlog();
#endif

	// Use this two line to test the crash handler...
//	int* crash = nullptr;
//	*crash = 0;

	XournalMain* main = new XournalMain();
	int result = main->run(argc, argv);
	delete main;

#ifdef DEV_CALL_LOG
	Log::closelog();
#endif

	return result;
}
