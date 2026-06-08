#include "CustomExportJob.h"

#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <utility>   // for move, pair
#include <vector>    // for vector

#include <gtk/gtk.h>  // for GTK_WINDOW

#include "control/Control.h"                   // for Control
#include "control/jobs/BaseExportJob.h"        // for BaseExportJob::ExportType
#include "control/xojfile/XojExportHandler.h"  // for XojExportHandler
#include "gui/MainWindow.h"                    // for MainWindow
#include "gui/dialog/ExportDialog.h"           // for ExportDialog
#include "model/Document.h"                    // for Document
#include "pdf/base/XojPdfExport.h"             // for XojPdfExport
#include "pdf/base/XojPdfExportFactory.h"      // for XojPdfExportFactory
#include "util/PathUtil.h"                     // for clearExtensions
#include "util/PopupWindowWrapper.h"           // for PopupWindowWrapper
#include "util/Util.h"                         // for execInUiThread
#include "util/XojMsgBox.h"                    // for XojMsgBox
#include "util/i18n.h"                         // for _, FS, _F
#include "util/raii/GtkWindowUPtr.h"           // for GtkWindowUPtr

#include "ImageExport.h"  // for ImageExport, EXPORT_GR...
#include "SaveJob.h"      // for SaveJob
#include "XournalScheduler.h"

namespace {

class ExportFormatDialog {
public:
    ExportFormatDialog(std::vector<std::string> filterNames, std::string selectedFilterName,
                       std::function<void(std::optional<std::string>)> callback):
            filterNames(std::move(filterNames)), callback(std::move(callback)) {
        GtkWidget* dialog = gtk_dialog_new_with_buttons(_("Export Format"), nullptr, GTK_DIALOG_MODAL, _("_Cancel"),
                                                        GTK_RESPONSE_CANCEL, _("Next"), GTK_RESPONSE_OK, nullptr);
        window.reset(GTK_WINDOW(dialog));

        auto* contentArea = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        gtk_container_set_border_width(GTK_CONTAINER(box), 12);
        gtk_box_pack_start(GTK_BOX(contentArea), box, TRUE, TRUE, 0);

        GtkWidget* label = gtk_label_new(_("Choose export format:"));
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

        comboBox = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
        int activeIndex = 0;
        for (size_t i = 0; i < this->filterNames.size(); i++) {
            gtk_combo_box_text_append_text(comboBox, this->filterNames[i].c_str());
            if (this->filterNames[i] == selectedFilterName) {
                activeIndex = static_cast<int>(i);
            }
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(comboBox), activeIndex);
        gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(comboBox), FALSE, FALSE, 0);

        gtk_widget_show_all(contentArea);

        signalId = g_signal_connect(
                dialog, "response", G_CALLBACK(+[](GtkDialog*, int response, gpointer data) {
                    auto* self = static_cast<ExportFormatDialog*>(data);
                    std::optional<std::string> selected;
                    if (response == GTK_RESPONSE_OK) {
                        int activeIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(self->comboBox));
                        if (activeIndex >= 0 && static_cast<size_t>(activeIndex) < self->filterNames.size()) {
                            selected = self->filterNames[activeIndex];
                        }
                    }

                    auto callback = self->takeCallback();
                    g_signal_handler_disconnect(self->window.get(), self->signalId);
                    gtk_window_close(self->window.get());
                    if (callback) {
                        callback(std::move(selected));
                    }
                }),
                this);

#if GTK_MAJOR_VERSION == 3
        g_signal_connect(dialog, "delete-event", G_CALLBACK(+[](GtkWidget*, GdkEvent*, gpointer data) -> gboolean {
                             auto* self = static_cast<ExportFormatDialog*>(data);
                             auto callback = self->takeCallback();
                             if (callback) {
                                 callback(std::nullopt);
                             }
                             return false;
                         }),
                         this);
#else
        g_signal_connect(dialog, "close-request", G_CALLBACK(+[](GtkWindow*, gpointer data) -> gboolean {
                             auto* self = static_cast<ExportFormatDialog*>(data);
                             auto callback = self->takeCallback();
                             if (callback) {
                                 callback(std::nullopt);
                             }
                             return false;
                         }),
                         this);
#endif
    }

    GtkWindow* getWindow() const { return window.get(); }

private:
    std::function<void(std::optional<std::string>)> takeCallback() {
        if (completed) {
            return {};
        }
        completed = true;
        return std::move(callback);
    }

    xoj::util::GtkWindowUPtr window;
    GtkComboBoxText* comboBox = nullptr;
    std::vector<std::string> filterNames;
    std::function<void(std::optional<std::string>)> callback;
    gulong signalId = 0;
    bool completed = false;
};

}  // namespace


CustomExportJob::CustomExportJob(Control* control): BaseExportJob(control, _("Custom Export")) {
    // Supported filters. Only extensions are stored: export save dialogs are native file
    // choosers, and their Win32 backend cannot translate MIME types.
    chosenFilterName = _("PDF files");
    filters.insert({chosenFilterName, ExportType(".pdf")});
    filters.insert({_("PNG graphics"), ExportType(".png")});
    filters.insert({_("SVG graphics"), ExportType(".svg")});
    filters.insert({_("Xournal (Compatibility)"), ExportType(".xoj")});
}

CustomExportJob::~CustomExportJob() = default;

void CustomExportJob::addFilterToDialog(GtkFileChooser* dialog) {
    const auto& chosenFilter = filters.at(chosenFilterName);
    addFileFilterToDialog(dialog, chosenFilterName, chosenFilter.extension);
}

void CustomExportJob::setExtensionFromFilter(fs::path& file, const char* /*filterName*/) const {
    // Native file choosers do not reliably report the active filter after selection.
    // Custom export chooses the format before opening the file chooser, so use that
    // explicit choice here instead of gtk_file_chooser_get_filter().
    const auto& chosenFilter = filters.at(chosenFilterName);

    // Remove any pre-existing extension and adds the chosen one
    Util::clearExtensions(file, chosenFilter.extension);
    file += chosenFilter.extension;
}

void CustomExportJob::showDialogAndRun() {
    std::vector<std::string> filterNames;
    for (const auto& [filterName, _]: filters) {
        filterNames.push_back(filterName);
    }

    auto onFileSelected = [job = this]() {
        Util::execInUiThread([job]() {
            if (job->exportTypeXoj) {
                job->control->getScheduler()->addJob(job, JOB_PRIORITY_NONE);
                return;
            }

            auto* ctrl = job->control;
            xoj::popup::PopupWindowWrapper<xoj::popup::ExportDialog> popup(
                    ctrl->getGladeSearchPath(), job->format, ctrl->getCurrentPageNo() + 1,
                    ctrl->getDocument()->getPageCount(), !ctrl->getDocument()->getPdfFilepath().empty(),
                    [job](const xoj::popup::ExportDialog& dialog) {
                        if (dialog.isConfirmed()) {
                            job->exportRange = dialog.getRange();
                            job->progressiveMode = dialog.progressiveModeSelected();
                            job->exportBackground = dialog.getBackgroundType();
                            job->pdfExportBackend = dialog.getPdfExportBackend();

                            if (job->format == EXPORT_GRAPHICS_PNG) {
                                job->pngQualityParameter = dialog.getPngQualityParameter();
                            }

                            job->control->getScheduler()->addJob(job, JOB_PRIORITY_NONE);
                        } else {
                            // The job blocked, so we have to unblock, because the job
                            // unblocks only after run
                            job->control->unblock();
                        }
                        job->unref();
                    });
            popup.show(GTK_WINDOW(job->control->getWindow()->getWindow()));
        });
    };

    auto onCancel = [job = this]() {
        job->control->unblock();
        job->unref();
    };

    xoj::popup::PopupWindowWrapper<ExportFormatDialog> popup(
            std::move(filterNames), chosenFilterName,
            [job = this, onFileSelected = std::move(onFileSelected),
             onCancel = std::move(onCancel)](std::optional<std::string> selectedFilterName) mutable {
                if (!selectedFilterName) {
                    onCancel();
                    return;
                }

                job->chosenFilterName = std::move(*selectedFilterName);
                const auto& chosenFilter = job->filters.at(job->chosenFilterName);
                job->exportTypeXoj = chosenFilter.extension == ".xoj";

                if (chosenFilter.extension == ".pdf") {
                    job->format = EXPORT_GRAPHICS_PDF;
                } else if (chosenFilter.extension == ".svg") {
                    job->format = EXPORT_GRAPHICS_SVG;
                } else if (chosenFilter.extension == ".png") {
                    job->format = EXPORT_GRAPHICS_PNG;
                } else {
                    job->format = EXPORT_GRAPHICS_UNDEFINED;
                }

                job->BaseExportJob::showFileChooser(std::move(onFileSelected), std::move(onCancel));
            });
    popup.show(GTK_WINDOW(control->getWindow()->getWindow()));
}

/**
 * Create one Graphics file per page
 */
void CustomExportJob::exportGraphics() {
    ImageExport imgExport(control->getDocument(), filepath, format, exportBackground, exportRange);
    if (format == EXPORT_GRAPHICS_PNG) {
        imgExport.setQualityParameter(pngQualityParameter);
    }
    imgExport.exportGraphics(control);
    errorMsg = imgExport.getLastErrorMsg();
}

void CustomExportJob::run() {
    if (exportTypeXoj) {
        SaveJob::updatePreview(control);
        Document* doc = this->control->getDocument();

        XojExportHandler h;
        doc->lock_shared();
        h.prepareSave(doc, filepath);
        h.saveTo(filepath, this->control);
        doc->unlock_shared();

        if (!h.getErrorMessage().empty()) {
            this->lastError = FS(_F("Save file error: {1}") % h.getErrorMessage());

            callAfterRun();
        }
    } else if (format == EXPORT_GRAPHICS_PDF) {
        // don't lock the page here for the whole flow, else we get a dead lock...
        // the ui is blocked, so there should be no changes...
        Document* doc = control->getDocument();

        std::unique_ptr<XojPdfExport> pdfe = XojPdfExportFactory::createExport(doc, control, pdfExportBackend);

        pdfe->setExportBackground(exportBackground);

        if (!pdfe->createPdf(this->filepath, exportRange, progressiveMode)) {
            this->errorMsg = pdfe->getLastError();
        }

    } else {
        exportGraphics();
    }
}

void CustomExportJob::afterRun() {
    if (!this->lastError.empty()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), this->lastError);
    }
}
