#include "CrashHandler.h"

#include <string>  // for string

#include <glib.h>  // for g_warning, g_error

#include "control/xojfile/SaveHandler.h"  // for SaveHandler
#include "util/PathUtil.h"                // for getConfigFile
#include "util/i18n.h"                    // for FC, _F, _

#include "filesystem.h"  // for path

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

    auto const& filepath = Util::getConfigFile("emergencysave.xopp");

    SaveHandler handler;
    handler.prepareSave(document);
    handler.saveTo(filepath);

    if (!handler.getErrorMessage().empty()) {
        g_error("%s", FC(_F("Error: {1}") % handler.getErrorMessage()));
    } else {
        g_warning("%s", FC(_F("Successfully saved document to \"{1}\"") % filepath.string()));
    }
}
