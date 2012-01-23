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
#include <cxxabi.h>

#include "CrashHandler.h"
#include "../control/SaveHandler.h"
#include "../model/Document.h"
#include "Stacktrace.h"

#include "../cfg.h"

static bool alreadyCrashed = false;
static Document * document = NULL;

void setEmergencyDocument(Document * doc) {
	document = doc;
}

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

static void emergencySave();

/**
 * Print crash log to config directory
 */
static void crashHandler(int sig) {
	if (alreadyCrashed) { // crasehd again on emergency save
		exit(2);
	}
	alreadyCrashed = true;

	fprintf(stderr, "Crash Handler::Crashed with signal %i\n", sig);

	time_t lt;
	void *array[100];
	char ** messages;

	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 100);

	gchar *filename = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, "errorlog.log", NULL);

	FILE * fp = fopen(filename, "w");
	if (fp) {
		fprintf(stderr, "Crash Handler::wrote crash log to: %s\n", filename);
	}

	lt = time(NULL);

	fprintf(fp, "Date: %s\n", ctime(&lt));
	fprintf(stderr, "Date: %s\n", ctime(&lt));
	fprintf(fp, "Error: signal %d:\n", sig);
	fprintf(stderr, "Error: signal %d:\n", sig);

	messages = backtrace_symbols(array, size);

	for (int i = 0; i < size; i++) {
		fprintf(fp, "[bt]: (%d) %s\n", i, messages[i]);
		fprintf(stderr, "[bt]: (%d) %s\n", i, messages[i]);
	}

	g_free(filename);
	free(messages);

	fprintf(fp, "\n\nTry to get a better stracktrace...\n");
	fprintf(stderr, "\n\nTry to get a better stracktrace...\n");

	Stacktrace::printStracktrace(fp);
	Stacktrace::printStracktrace(stderr);

	if (fp) {
		fclose(fp);
	}

	emergencySave();

	exit(1);
}

static void emergencySave() {
	if (document == NULL) {
		return;
	}

	fprintf(stderr, "\nTry to emergency save the current open document...\n");

	SaveHandler handler;
	handler.prepareSave(document);

	gchar * filename = g_strconcat(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR, G_DIR_SEPARATOR_S, "emergencysave.xoj", NULL);

	GzOutputStream * out = new GzOutputStream(filename);

	if (!out->getLastError().isEmpty()) {
		fprintf(stderr, "error: %s\n", out->getLastError().c_str());
		delete out;
		g_free(filename);
		return;
	}

	handler.saveTo(out, filename);
	out->close();

	if (!out->getLastError().isEmpty()) {
		fprintf(stderr, "error: %s\n", out->getLastError().c_str());
	} else {
		fprintf(stderr, "Successfully saved document to \"%s\"\n", filename);
	}

	g_free(filename);
	delete out;
}

