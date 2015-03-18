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
#include <iostream>
#include <fstream>
#include <sstream>
#include <execinfo.h>

#include "CrashHandler.h"
#include "../control/SaveHandler.h"
#include "../model/Document.h"
#include "Stacktrace.h"

#include "../cfg.h"

using namespace std;

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

class streamsplit : public stringstream {
public:
    streamsplit(ofstream* file) {
        f = file;
    }
    
    template<typename T>
    ostream& operator<<(T const & rhs) {
        *f << rhs;
        cerr << rhs;
        return *f;
    }
    
private:
    ofstream* f;
};

/**
 * Print crash log to config directory
 */
static void crashHandler(int sig) {
    if (alreadyCrashed) { // crasehd again on emergency save
        exit(2);
    }
    alreadyCrashed = true;

    cerr << "Crash Handler::Crashed with signal " << sig << "\n";

    time_t lt;
    void* array[100];
    char** messages;

    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    String* filename = CONCAT(g_get_home_dir(), G_DIR_SEPARATOR_S, CONFIG_DIR,
            G_DIR_SEPARATOR_S, "errorlog.log");

    ofstream fp(CSTR(*filename));
    if (fp) cerr << "Crash Handler::wrote crash log to: " << *filename << endl;
    
    streamsplit out(&fp);

    lt = time(NULL);
    
    out << "Date: " << ctime(&lt) << endl
        << "Error: signal " << sig << endl;

    messages = backtrace_symbols(array, size);

    for (int i = 0; i < size; i++) {
        out << "[bt]: (" << i << ") " << messages[i] << endl;
    }

    delete filename;
    free(messages);

    out << "\n\nTry to get a better stracktrace...\n";

    Stacktrace::printStracktrace(out);
    
    if (fp) {
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

	cerr << "\nTry to emergency save the current open document...\n";

	SaveHandler handler;
	handler.prepareSave(document);

	String* filename = CONCAT(g_get_home_dir(), G_DIR_SEPARATOR_S,
                                    CONFIG_DIR, G_DIR_SEPARATOR_S,
                                    "emergencysave.xoj");

	GzOutputStream* out = new GzOutputStream(*filename);

	if (!out->getLastError().isEmpty())
	{
		cerr << "error: " << out->getLastError() << endl;
		delete out;
		delete filename;
		return;
	}

	handler.saveTo(out, *filename);
	out->close();

	if (!out->getLastError().isEmpty())
	{
		cerr << "error: " << out->getLastError() << endl;
	}
	else
	{
		cerr << "Successfully saved document to \"" << *filename << "\"" << endl;
	}

	delete filename;
	delete out;
}

