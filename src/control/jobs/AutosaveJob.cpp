#include "AutosaveJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"

#include "XojMsgBox.h"
#include "filesystem.h"
#include "i18n.h"

AutosaveJob::AutosaveJob(Control* control): control(control) {}

AutosaveJob::~AutosaveJob() = default;

void AutosaveJob::afterRun() {
    string msg = FS(_F("Error while autosaving: {1}") % this->error);
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
        filepath = filepath.parent_path() / ("." + filepath.filename().string());
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
