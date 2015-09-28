#include "CrashHandler.h"

#include "Stacktrace.h"
#include "Util.h"

#include "control/xojfile/SaveHandler.h"
#include "model/Document.h"

#include <config-dev.h>

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

/* 
 * Basicly it's really simple class and you have to know what you can and what
 * you can't do with it. So please don't touch anythink down there without
 * previous consultation.
 */
class streamsplit : public std::stringstream
{
public:

	streamsplit(std::ofstream* file)
	{
		f = file;
	}

	template<typename T>
	std::ostream& operator<<(T const & rhs)
	{
		*f << rhs;
		cerr << rhs;
		return *f;
	}

private:
	std::ofstream* f;
};

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

	cerr << bl::format("Crash Handler::Crashed with signal {1}") % sig << endl;

	time_t lt;
	void* array[100];
	char** messages;

	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 100);

	time_t curtime = time(0);
	char stime[128];
	strftime(stime, sizeof(stime), "%Y%m%d-%H%M%S", localtime(&curtime));
	string filename = Util::getConfigFile(CONCAT("errorlog.", stime, ".log")).string();
	ofstream fp(filename);
	if (fp) cerr << bl::format("Crash Handler::wrote crash log to: {1}") % filename << endl;

	streamsplit out(&fp);

	lt = time(NULL);

	out << bl::format("Date: {1}") % ctime(&lt);
	out << bl::format("Error: signal {1}") % sig;
	out << "\n";

	messages = backtrace_symbols(array, size);

	for (int i = 0; i < size; i++)
	{
		out << bl::format("[bt]: ({1}) {2}") % i % messages[i];
		out << "\n";
	}

	free(messages);

	out << "\n\nTry to get a better stracktrace...\n";

	Stacktrace::printStracktrace(out);

	if (fp) fp.close();

	emergencySave();

	exit(1);
}

static void emergencySave()
{
	if (document == NULL)
	{
		return;
	}

	cerr << endl << "Try to emergency save the current open document..." << endl;

	SaveHandler handler;
	handler.prepareSave(document);

	path filename = Util::getConfigFile("emergencysave.xoj");

	GzOutputStream* out = new GzOutputStream(filename);

	if (!out->getLastError().empty())
	{
		cerr << bl::format("error: {1}") % out->getLastError() << endl;
		delete out;
		return;
	}

	handler.saveTo(out, filename);
	out->close();

	if (!out->getLastError().empty())
	{
		cerr << bl::format("error: {1}") % out->getLastError() << endl;
	}
	else
	{
		cerr << bl::format("Successfully saved document to \"{1}\"") % filename.string() << endl;
	}

	delete out;
}
