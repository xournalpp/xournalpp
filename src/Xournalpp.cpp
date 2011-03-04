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

int main(int argc, char *argv[]) {
	// init crash handler
	installCrashHandlers();
	if (argc) {
		Stacktrace::setExename(argv[0]);
	}

	XournalMain main;
	return main.run(argc, argv);
}
