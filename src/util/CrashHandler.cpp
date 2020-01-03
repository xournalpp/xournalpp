#include "CrashHandler.h"

#include <ctime>

#include <config-dev.h>
#include <gtk/gtk.h>

#include "control/xojfile/SaveHandler.h"
#include "model/Document.h"

#include "Stacktrace.h"
#include "Util.h"
#include "i18n.h"

static Document* document = nullptr;

void setEmergencyDocument(Document* doc) { document = doc; }

#ifdef _WIN32
#include "CrashHandlerWindows.h"
#else
#include "CrashHandlerUnix.h"
#endif

void emergencySave() {
    if (document == nullptr) {
        return;
    }

    g_warning(_("Trying to emergency save the current open document…"));

    Path filename = Util::getConfigFile("emergencysave.xopp");

    SaveHandler handler;
    handler.prepareSave(document);
    handler.saveTo(filename);

    if (!handler.getErrorMessage().empty()) {
        g_error("%s", FC(_F("Error: {1}") % handler.getErrorMessage()));
    } else {
        g_warning("%s", FC(_F("Successfully saved document to \"{1}\"") % filename.str()));
    }
}
