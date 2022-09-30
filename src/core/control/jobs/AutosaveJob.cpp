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
    XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
}

void AutosaveJob::run() {
    SaveHandler handler;

    control->getUndoRedoHandler()->documentAutosaved();

    Document* doc = control->getDocument();

    doc->lock();
    auto filepath = doc->getFilepath();

    if (filepath.empty()) {
        filepath = Util::getAutosaveFilepath();
    } else {
        filepath.replace_filename(fs::u8path(u8"." + filepath.filename().u8string()));
    }
    Util::clearExtensions(filepath);
    filepath += ".autosave.xopp";

    handler.prepareSave(doc, filepath);
    doc->unlock();

    g_message("%s", FS(_F("Autosaving to {1}") % filepath.string()).c_str());

    fs::path tempfile = filepath;
    tempfile += u8"~";
    handler.saveTo(tempfile);

    this->error = handler.getErrorMessage();
    if (!this->error.empty()) {
        callAfterRun();
    } else {
        try {
            if (fs::exists(filepath)) {
                fs::path swaptmpfile = filepath;
                swaptmpfile += u8".swap";
                Util::safeRenameFile(filepath, swaptmpfile);
                Util::safeRenameFile(tempfile, filepath);
                // All went well, we can delete the old autosave file
                fs::remove(swaptmpfile);
            } else {
                Util::safeRenameFile(tempfile, filepath);
            }
            control->setLastAutosaveFile(filepath);
        } catch (const fs::filesystem_error& e) {
            auto fmtstr = _F("Could not rename autosave file from \"{1}\" to \"{2}\": {3}");
            this->error = FS(fmtstr % tempfile.u8string() % filepath.u8string() % e.what());
        }
    }
}

auto AutosaveJob::getType() -> JobType { return JOB_TYPE_AUTOSAVE; }
