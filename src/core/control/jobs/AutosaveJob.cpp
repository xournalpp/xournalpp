#include "AutosaveJob.h"

#include <glib.h>  // for g_message, g_warning

#include "control/Control.h"              // for Control
#include "control/jobs/Job.h"             // for JOB_TYPE_AUTOSAVE, JobType
#include "control/xojfile/SaveHandler.h"  // for SaveHandler
#include "model/Document.h"               // for Document
#include "undo/UndoRedoHandler.h"         // for UndoRedoHandler
#include "util/PathUtil.h"                // for clearExtensions, getAutosav...
#include "util/XojMsgBox.h"               // for XojMsgBox
#include "util/i18n.h"                    // for FS, _F

#include "filesystem.h"  // for path, u8path

AutosaveJob::AutosaveJob(Control* control): control(control) {}

AutosaveJob::~AutosaveJob() = default;

void AutosaveJob::afterRun() {
    std::string msg = FS(_F("Error while autosaving: {1}") % this->error);
    g_warning("%s", msg.c_str());
    XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
}

void AutosaveJob::run() {
    SaveHandler handler;

    control->getUndoRedoHandler()->documentAutosaved();

    Document* doc = control->getDocument();

    doc->lock();
    handler.prepareSave(doc);
    auto filepath = doc->getFilepath();
    doc->unlock();

    if (filepath.empty()) {
        filepath = Util::getAutosaveFilepath();
    } else {
        filepath.replace_filename(fs::u8path(u8"." + filepath.filename().u8string()));
    }
    Util::clearExtensions(filepath);
    filepath += ".autosave.xopp";

    control->renameLastAutosaveFile();

    g_message("%s", FS(_F("Autosaving to {1}") % filepath.string()).c_str());

    handler.saveTo(filepath);

    this->error = handler.getErrorMessage();
    if (!this->error.empty()) {
        callAfterRun();
    } else {
        // control->deleteLastAutosaveFile(filepath);
        control->setLastAutosaveFile(filepath);
    }
}

auto AutosaveJob::getType() -> JobType { return JOB_TYPE_AUTOSAVE; }
