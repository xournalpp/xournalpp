#include "CrashHandler.h"

#include "Stacktrace.h"
#include "Util.h"

#include "control/xojfile/SaveHandler.h"
#include "model/Document.h"

#include <config-dev.h>
#include <i18n.h>

#include <glib.h>
#include <gtk/gtk.h>

#include <boost/locale/format.hpp>

using std::to_string;
#include <ctime>
#include <iostream>
using std::cerr;
using std::endl;
#include <fstream>
using std::ofstream;
#include <sstream>
#include <execinfo.h>

static bool alreadyCrashed = false;
static Document* document = NULL;

void setEmergencyDocument(Document* doc)
{
	document = doc;
}

static void crashHandler(int sig);

void installCrashHandlers()
{
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
static void crashHandler(int sig)
{
	if (alreadyCrashed) // crasehd again on emergency save
	{
		exit(2);
	}
	alreadyCrashed = true;

	cerr << bl::format("[Crash Handler] Crashed with signal {1}") % sig << endl;

	time_t lt;
	void* array[100];
	char** messages;

	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 100);

	time_t curtime = time(0);
	char stime[128];
	strftime(stime, sizeof(stime), "%Y%m%d-%H%M%S", localtime(&curtime));
	string filename = Util::getConfigFile(std::string("errorlogs/errorlog.")  + stime + ".log").string();
	ofstream fp(filename);
	if (fp)
	{
		cerr << bl::format("[Crash Handler] Wrote crash log to: {1}") % filename << endl;
	}

	lt = time(NULL);

	fp << bl::format("Date: {1}") % ctime(&lt);
	fp << bl::format("Error: signal {1}") % sig;
	fp << "\n";

	messages = backtrace_symbols(array, size);

	for (size_t i = 0; i < size; i++)
	{
		fp << bl::format("[bt]: ({1}) {2}") % i % messages[i];
		fp << "\n";
	}

	free(messages);

	fp << "\n\nTry to get a better stracktrace...\n";

	Stacktrace::printStracktrace(fp);

	if (fp)
	{
		fp.close();
	}

	emergencySave();

	exit(1);
}

static void emergencySave()
{
	if (document == NULL)
	{
		return;
	}

	cerr << endl << _("Trying to emergency save the current open document…") << endl;

	SaveHandler handler;
	handler.prepareSave(document);

	path filename = Util::getConfigFile("emergencysave.xopp");

	GzOutputStream* out = new GzOutputStream(filename);

	if (!out->getLastError().empty())
	{
		cerr << _F("Error: {1}") % out->getLastError() << endl;
		delete out;
		return;
	}

	handler.saveTo(out, filename);
	out->close();

	if (!out->getLastError().empty())
	{
		cerr << _F("Error: {1}") % out->getLastError() << endl;
	}
	else
	{
		cerr << _F("Successfully saved document to \"{1}\"") % filename.string() << endl;
	}

	delete out;
}
