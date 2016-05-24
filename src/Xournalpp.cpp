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
	if (argc)
	{
		// Filename is needed to get backtracke with filenumbers
		Stacktrace::setExename(argv[0]);
	}

#ifdef DEV_CALL_LOG
	Log::initlog();
#endif

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
