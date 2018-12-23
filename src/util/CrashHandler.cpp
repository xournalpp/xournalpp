#include "CrashHandler.h"

#include "Stacktrace.h"
#include "Util.h"

#include "control/xojfile/SaveHandler.h"
#include "model/Document.h"

#include <config-dev.h>
#include <i18n.h>

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

static Document* document = NULL;

void setEmergencyDocument(Document* doc)
{
	document = doc;
}

#ifdef WIN32
#include "CrashHandlerWindows.h"
#else
#include "CrashHandlerUnix.h"
#endif

void emergencySave()
{
	if (document == NULL)
	{
		return;
	}

	cerr << endl << _("Trying to emergency save the current open document…") << endl;

	path filename = Util::getConfigFile("emergencysave.xopp");

	SaveHandler handler;
	handler.prepareSave(document);
	handler.saveTo(filename);

	if (!handler.getErrorMessage().empty())
	{
		cerr << _F("Error: {1}") % handler.getErrorMessage() << endl;
	}
	else
	{
		cerr << _F("Successfully saved document to \"{1}\"") % filename.string() << endl;
	}
}
