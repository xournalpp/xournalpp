#include "BaseExportJob.h"

#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "control/Control.h"            // for Control
#include "control/jobs/BlockingJob.h"   // for BlockingJob
#include "control/settings/Settings.h"  // for Settings
#include "gui/MainWindow.h"             // for MainWindow
#include "model/Document.h"             // for Document, Document::PDF
#include "util/PathUtil.h"              // for toGFilename, clearExtensions
#include "util/PopupWindowWrapper.h"    // for PopupWindowWrapper
#include "util/XojMsgBox.h"             // for XojMsgBox
#include "util/glib_casts.h"            // for wrap_for_g_callback_v
#include "util/i18n.h"                  // for _, FS, _F

BaseExportJob::BaseExportJob(Control* control, const std::string& name): BlockingJob(control, name) {}

BaseExportJob::~BaseExportJob() = default;

void BaseExportJob::addFileFilterToDialog(GtkFileChooser* dialog, const std::string& name, const std::string& mime) {
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, name.c_str());
    gtk_file_filter_add_mime_type(filter, mime.c_str());
    gtk_file_chooser_add_filter(dialog, filter);
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
            std::string msg = _("Do not overwrite the background PDF! This will cause errors!");
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

void BaseExportJob::showFileChooser(std::function<void()> onFileSelected, std::function<void()> onCancel) {
    auto* dialog =
            gtk_file_chooser_dialog_new(_("Export"), control->getGtkWindow(), GTK_FILE_CHOOSER_ACTION_SAVE,
                                        _("_Cancel"), GTK_RESPONSE_CANCEL, _("_Export"), GTK_RESPONSE_OK, nullptr);
    addFilterToDialog(GTK_FILE_CHOOSER(dialog));

    Settings* settings = control->getSettings();
    Document* doc = control->getDocument();
    doc->lock();
    fs::path folder = doc->createSaveFolder(settings->getLastSavePath());
    fs::path name = doc->createSaveFilename(Document::PDF, settings->getDefaultSaveName(), settings->getDefaultPdfExportName());
    doc->unlock();

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Util::toGFile(folder).get(), nullptr);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), Util::toGFilename(name).c_str());

    class FileDlg final {
    public:
        FileDlg(GtkDialog* dialog, BaseExportJob* job, std::function<void()> onFileSelected,
                std::function<void()> onCancel):
                window(GTK_WINDOW(dialog)),
                job(job),
                onFileSelected(std::move(onFileSelected)),
                onCancel(std::move(onCancel)) {
            this->signalId = g_signal_connect(
                    dialog, "response", G_CALLBACK((+[](GtkDialog* dialog, int response, gpointer data) {
                        FileDlg* self = static_cast<FileDlg*>(data);
                        auto* job = self->job;
                        if (response == GTK_RESPONSE_OK) {
                            auto file = Util::fromGFile(
                                    xoj::util::GObjectSPtr<GFile>(gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog)),
                                                                  xoj::util::adopt)
                                            .get());
                            Util::clearExtensions(file);
                            // Since we add the extension after the OK button, we have to check manually on existing
                            // files
                            const char* filterName =
                                    gtk_file_filter_get_name(gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog)));
                            if (job->testAndSetFilepath(std::move(file), filterName)) {
                                auto doExport = [self, dialog](const fs::path& file) {
                                    // Closing the window causes another "response" signal, which we want to ignore
                                    g_signal_handler_disconnect(dialog, self->signalId);
                                    gtk_window_close(GTK_WINDOW(dialog));
                                    self->job->control->getSettings()->setLastSavePath(file.parent_path());
                                    self->onFileSelected();
                                };
                                XojMsgBox::replaceFileQuestion(GTK_WINDOW(dialog), job->filepath, std::move(doExport));
                            }  // else the dialog stays on until a suitable destination is found or cancel is hit.
                        } else {
                            self->onCancel();
                            // Closing the window causes another "response" signal, which we want to ignore
                            g_signal_handler_disconnect(dialog, self->signalId);
                            gtk_window_close(GTK_WINDOW(dialog));  // Deletes self, don't do anything after this
                        }
                    })),
                    this);
        }
        ~FileDlg() = default;

        inline GtkWindow* getWindow() const { return window.get(); }

    private:
        xoj::util::GtkWindowUPtr window;
        BaseExportJob* job;
        std::function<void()> onFileSelected;
        std::function<void()> onCancel;
        gulong signalId;
    };

    auto popup = xoj::popup::PopupWindowWrapper<FileDlg>(GTK_DIALOG(dialog), this, std::move(onFileSelected),
                                                         std::move(onCancel));
    popup.show(GTK_WINDOW(this->control->getWindow()->getWindow()));
}

auto BaseExportJob::testAndSetFilepath(const fs::path& file, const char* /*filterName*/) -> bool {
    try {
        if (fs::is_directory(file.parent_path())) {
            this->filepath = file;
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
