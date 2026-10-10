#include "BaseExportJob.h"

#include <glib.h>  // for g_warning

#include "control/Control.h"            // for Control
#include "control/jobs/BlockingJob.h"   // for BlockingJob
#include "model/Document.h"             // for Document
#include "util/XojMsgBox.h"             // for XojMsgBox
#include "util/i18n.h"                  // for _, FS, _F

BaseExportJob::BaseExportJob(Control* control, const std::string& name): BlockingJob(control, name) {}

BaseExportJob::~BaseExportJob() = default;

auto BaseExportJob::checkOverwriteBackgroundPDF(fs::path const& file) const -> bool {
    auto backgroundPDF = control->getDocument()->getPdfFilepath();
    try {
        if (backgroundPDF.empty() || !fs::exists(backgroundPDF)) {
            // If there is no background, we can return
            return true;
        }

        if (fs::weakly_canonical(file) == fs::weakly_canonical(backgroundPDF)) {
            // If the new file name (with the selected extension) is the previously selected pdf, warn the user
            std::string msg = _("Do not overwrite the background PDF! This will cause errors!");
            XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
            return false;
        }
    } catch (const fs::filesystem_error& fe) {
        g_warning("%s", fe.what());
        auto msg = std::string(_("The check for overwriting the background failed with:\n")) + fe.what();
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        return false;
    }
    return true;
}

auto BaseExportJob::testFilepath(const fs::path& file) const -> bool {
    try {
        if (!file.empty() && fs::is_directory(file.parent_path()) && checkOverwriteBackgroundPDF(file)) {
            return true;
        }
    } catch (const fs::filesystem_error& e) {
        std::string msg = FS(_F("Failed to resolve path with the following error:\n{1}") % e.what());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }
    return false;
}

void BaseExportJob::afterRun() {
    if (!this->errorMsg.empty()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), this->errorMsg);
    }
}
