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
#include "util/CrashHandler.h"
#include "util/Stacktrace.h"

int main(int argc, char * argv[]) {
	// init crash handler
	installCrashHandlers();
	if (argc) {
		// Filename is needed to get backtracke with filenumbers
		Stacktrace::setExename(argv[0]);
	}

	XournalMain * main = new XournalMain();
	int result = main->run(argc, argv);
	delete main;

#ifdef XOJ_MEMORY_LEAK_CHECK_ENABLED
	xoj_momoryleak_printRemainingObjects();
#endif

	return result;
}
