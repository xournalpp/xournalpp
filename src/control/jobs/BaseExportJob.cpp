#include "BaseExportJob.h"

#include <utility>

#include "control/Control.h"

#include "StringUtils.h"
#include "XojMsgBox.h"
#include "i18n.h"

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
    if (!fs::exists(backgroundPDF)) {
        return true;
    }
    // If the new file name (with the selected extension) is the previously selected pdf, warn the user
    if (fs::equivalent(file, backgroundPDF)) {
        string msg = _("Do not overwrite the background PDF! This will cause errors!");
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        return false;
    }
    return true;
}

auto BaseExportJob::getFilterName() -> string {
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
    fs::path name = doc->createSaveFilename(Document::PDF, settings->getDefaultSaveName());
    doc->unlock();

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), folder.string().c_str());
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), name.string().c_str());

    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(this->control->getWindow()->getWindow()));

    while (true) {
        if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK) {
            gtk_widget_destroy(dialog);
            return false;
        }
        auto uri = [](char* uri) {
            std::string ret{uri};
            g_free(uri);
            return ret;
        }(gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));
        this->filepath = Util::fromGtkFilename(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));
        Util::clearExtensions(this->filepath);
        // Since we add the extension after the OK button, we have to check manually on existing files
        if (isUriValid(uri) && control->askToReplace(filepath)) {
            break;
        }
    }

    settings->setLastSavePath(this->filepath.parent_path());

    gtk_widget_destroy(dialog);

    return true;
}

auto BaseExportJob::isUriValid(string& uri) -> bool {
    if (!StringUtils::startsWith(uri, "file://")) {
        string msg = FS(_F("Only local files are supported\nPath: {1}") % uri);
        XojMsgBox::showErrorToUser(control->getGtkWindow(), msg);
        return false;
    }

    return true;
}

void BaseExportJob::afterRun() {
    if (!this->errorMsg.empty()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), this->errorMsg);
    }
}
