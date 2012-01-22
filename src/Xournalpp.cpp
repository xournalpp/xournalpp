/*
 * Xournal++
 *
 * The main application
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include "control/XournalMain.h"
#include <CrashHandler.h>
#include <Stacktrace.h>

int main(int argc, char * argv[]) {
	// init crash handler
	installCrashHandlers();
	if (argc) {
		// Filename is needed to get backtracke with filenumbers
		Stacktrace::setExename(argv[0]);
	}

#ifdef XOJ_CALL_LOG_ENABLED
	Log::initlog();
#endif

	XournalMain * main = new XournalMain();
	int result = main->run(argc, argv);
	delete main;

#ifdef XOJ_MEMORY_LEAK_CHECK_ENABLED
	xoj_momoryleak_printRemainingObjects();
#endif

#ifdef XOJ_CALL_LOG_ENABLED
	Log::closelog();
#endif

	return result;
}
