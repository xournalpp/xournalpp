/*
 * Xournal++
 *
 * Error handler, prints a stacktrace if Xournal crashes
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

#include "CrashHandler.h"
#include "../cfg.h"

static void crashHandler(int sig);

void installCrashHandlers() {
	sigset_t mask;

	sigemptyset(&mask);

#ifdef SIGSEGV
	signal(SIGSEGV, crashHandler);
	sigaddset(&mask, SIGSEGV);
#endif

#ifdef SIGFPE
	signal(SIGFPE, crashHandler);
	sigaddset(&mask, SIGFPE);
#endif

#ifdef SIGILL
	signal(SIGILL, crashHandler);
	sigaddset(&mask, SIGILL);
#endif

#ifdef SIGABRT
	signal(SIGABRT, crashHandler);
	sigaddset(&mask, SIGABRT);
#endif

	sigprocmask(SIG_UNBLOCK, &mask, 0);

}

/**
 * Print crash log to config directory
 */
static void crashHandler(int sig) {
	printf("Crash Handler::Crashed with signal %i\n", sig);

	time_t lt;
	void *array[100];
	char ** messages;

	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 100);

	gchar *filename = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, "errorlog.log",
			NULL);

	FILE * fp = fopen(filename, "w");

	bool toStdout = false;
	if (!fp) {
		fp = stdout;
		toStdout = true;
	} else {
		printf("Crash Handler::wrote crash log to: %s\n", filename);
	}

	lt = time(NULL);

	fprintf(fp, "Date: %s\n", ctime(&lt));
	fprintf(fp, "Error: signal %d:\n", sig);

	messages = backtrace_symbols(array, size);

	for (int i = 0; i < size; i++) {
		fprintf(fp, "[bt]: (%d) %s\n", i, messages[i]);
	}

	if (!toStdout) {
		fclose(fp);
	}
	g_free(filename);
	free(messages);

	exit(1);
}

