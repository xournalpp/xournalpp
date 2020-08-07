#include <filesystem>

#include "AutosaveJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"

#include "XojMsgBox.h"
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
    std::filesystem::path filename = doc->getFilename();
    doc->unlock();

    if (filename.empty()) {
        filename = Util::getAutosaveFilename();
    } else {
        string file = filename.filename();
        filename = filename.parent_path();
        filename /= string(".") + file;
    }
    filename.replace_extension(".autosave.xopp");

    control->renameLastAutosaveFile();

    g_message("%s", FS(_F("Autosaving to {1}") % filename.string()).c_str());

    handler.saveTo(filename);

    this->error = handler.getErrorMessage();
    if (!this->error.empty()) {
        callAfterRun();
    } else {
        // control->deleteLastAutosaveFile(filename);
        control->setLastAutosaveFile(filename);
    }
}

auto AutosaveJob::getType() -> JobType { return JOB_TYPE_AUTOSAVE; }
