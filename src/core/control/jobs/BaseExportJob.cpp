#include "BaseExportJob.h"

#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "control/Control.h"            // for Control
#include "control/jobs/BlockingJob.h"   // for BlockingJob
#include "control/settings/Settings.h"  // for Settings
#include "gui/MainWindow.h"             // for MainWindow
#include "model/Document.h"             // for Document, Document::PDF
#include "util/PathUtil.h"              // for toGFilename, clearExtensions
#include "util/XojMsgBox.h"             // for XojMsgBox
#include "util/i18n.h"                  // for _, FS, _F

using std::string;

BaseExportJob::BaseExportJob(Control* control, const string& name): BlockingJob(control, name) {}

BaseExportJob::~BaseExportJob() = default;

void BaseExportJob::initDialog() {
    dialog = gtk_file_chooser_dialog_new(_("Export PDF"), control->getGtkWindow(), GTK_FILE_CHOOSER_ACTION_SAVE,
                                         _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Save"), GTK_RESPONSE_OK, nullptr);

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
}

void BaseExportJob::addFileFilterToDialog(const string& name, const string& pattern) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, name.c_str());
    gtk_file_filter_add_pattern(filter, pattern.c_str());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
}

auto BaseExportJob::checkOverwriteBackgroundPDF(fs::path const& file) const -> bool {
    auto backgroundPDF = control->getDocument()->getPdfFilepath();
    // If there is no background, we can return
    try {
        if (!fs::exists(backgroundPDF)) {
            return true;
        }
        // If the new file name (with the selected extension) is the previously selected pdf, warn the user

        if (fs::weakly_canonical(file) == fs::weakly_canonical(backgroundPDF)) {
            string msg = _("Do not overwrite the background PDF! This will cause errors!");
            XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
            return false;
        }
    } catch (const fs::filesystem_error& fe) {
        g_warning("%s", fe.what());
        auto msg = std::string(_("The check for overwriting the background failed with:\n")) + fe.what() +
                   _("\n Do you want to continue?");
        return XojMsgBox::replaceFileQuestion(control->getGtkWindow(), msg) == GTK_RESPONSE_OK;
    }
    return true;
}

auto BaseExportJob::getFilterName() const -> string {
    GtkFileFilter* filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
    return gtk_file_filter_get_name(filter);
}

auto BaseExportJob::showFilechooser() -> bool {
    initDialog();
    addFilterToDialog();

    Settings* settings = control->getSettings();
    Document* doc = control->getDocument();
    doc->lock();
    fs::path folder = doc->createSaveFolder(settings->getLastSavePath());
    fs::path name = doc->createSaveFilename(Document::PDF, settings->getDefaultSaveName(), settings->getDefaultPdfExportName());
    doc->unlock();

    gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), true);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFilename(folder).c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), Util::toGFilename(name).c_str());

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));

    while (true) {
        if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            return false;
        }
        auto file = Util::fromGFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        Util::clearExtensions(file);
        // Since we add the extension after the OK button, we have to check manually on existing files
        if (testAndSetFilepath(std::move(file)) && control->askToReplace(this->filepath)) {
            break;
        }
    }

    settings->setLastSavePath(this->filepath.parent_path());

    gtk_widget_destroy(dialog);

    return true;
}

auto BaseExportJob::testAndSetFilepath(const fs::path& file) -> bool {
    try {
        if (fs::is_directory(file.parent_path())) {
            this->filepath = file;
            return true;
        }
    } catch (const fs::filesystem_error& e) {
        string msg = FS(_F("Failed to resolve path with the following error:\n{1}") % e.what());
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
    }
    return false;
}

void BaseExportJob::afterRun() {
    if (!this->errorMsg.empty()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), this->errorMsg);
    }
}
