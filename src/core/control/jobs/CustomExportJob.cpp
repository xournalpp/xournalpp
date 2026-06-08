#include "CustomExportJob.h"

#include <memory>   // for unique_ptr
#include <utility>  // for move, pair

#include <gtk/gtk.h>  // for GTK_WINDOW

#include "control/Control.h"                   // for Control
#include "control/jobs/BaseExportJob.h"        // for BaseExportJob::ExportType
#include "control/xojfile/XojExportHandler.h"  // for XojExportHandler
#include "gui/MainWindow.h"                    // for MainWindow
#include "gui/dialog/ExportDialog.h"           // for ExportDialog
#include "gui/dialog/FileChooserFiltersHelper.h"
#include "model/Document.h"                // for Document
#include "pdf/base/XojPdfExport.h"         // for XojPdfExport
#include "pdf/base/XojPdfExportFactory.h"  // for XojPdfExportFactory
#include "util/PathUtil.h"                 // for clearExtensions
#include "util/PopupWindowWrapper.h"       // for PopupWindowWrapper
#include "util/Util.h"                     // for execInUiThread
#include "util/XojMsgBox.h"                // for XojMsgBox
#include "util/i18n.h"                     // for _, FS, _F

#include "ImageExport.h"  // for ImageExport, EXPORT_GR...
#include "SaveJob.h"      // for SaveJob
#include "XournalScheduler.h"

namespace {
constexpr int FILTER_RESPONSE_OFFSET = 1000;
}

CustomExportJob::CustomExportJob(Control* control): BaseExportJob(control, _("Custom Export")) {
    // Supported filters
    filters.insert({_("PDF files"), ExportType(".pdf", "application/pdf")});
    filters.insert({_("PNG graphics"), ExportType(".png", "image/png")});
    filters.insert({_("SVG graphics"), ExportType(".svg", "image/svg+xml")});
    filters.insert({_("Xournal (Compatibility)"), ExportType(".xoj", "application/x-xojpp")});
}

CustomExportJob::~CustomExportJob() = default;

void CustomExportJob::addFilterToDialog(GtkFileChooser* dialog) {
    if (xoj::useNativeFileChooser() && !chosenFilterName.empty()) {
        const auto& filter = filters.at(chosenFilterName);
        addFileFilterToDialog(dialog, chosenFilterName, filter.mimeType, filter.extension);
        return;
    }

    // Runs on every filter inside the filters map
    for (auto& filter: filters) {
        addFileFilterToDialog(dialog, filter.first, filter.second.mimeType, filter.second.extension);
    }
}

void CustomExportJob::setExtensionFromFilter(fs::path& file, const char* filterName) const {
    if (!chosenFilterName.empty()) {
        filterName = chosenFilterName.c_str();
    }

    // Extract the file filter selected
    const auto& chosenFilter = filters.at(filterName);

    // Remove any pre-existing extension and adds the chosen one
    Util::clearExtensions(file, chosenFilter.extension);
    file += chosenFilter.extension;
}

void CustomExportJob::showDialogAndRun() {
    if (xoj::useNativeFileChooser()) {
        GtkWidget* dialog =
                gtk_message_dialog_new(GTK_WINDOW(control->getWindow()->getWindow()), GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION, GTK_BUTTONS_CANCEL, "%s", _("Select export format"));
        int response = FILTER_RESPONSE_OFFSET;
        for (const auto& filter: filters) {
            gtk_dialog_add_button(GTK_DIALOG(dialog), filter.first.c_str(), response++);
        }

        response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        if (response < FILTER_RESPONSE_OFFSET) {
            control->unblock();
            unref();
            return;
        }

        auto filter = std::next(filters.begin(), response - FILTER_RESPONSE_OFFSET);
        if (filter == filters.end()) {
            control->unblock();
            unref();
            return;
        }
        chosenFilterName = filter->first;
    }

    auto onFileSelected = [job = this]() {
        Util::execInUiThread([job]() {
            if (job->filepath.extension() == ".xoj") {
                job->exportTypeXoj = true;
                job->control->getScheduler()->addJob(job, JOB_PRIORITY_NONE);
                return;
            }

            if (auto ext = job->filepath.extension(); ext == ".pdf") {
                job->format = EXPORT_GRAPHICS_PDF;
            } else if (ext == ".svg") {
                job->format = EXPORT_GRAPHICS_SVG;
            } else if (ext == ".png") {
                job->format = EXPORT_GRAPHICS_PNG;
            } else {
                g_warning("Unknown extension");
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

    BaseExportJob::showFileChooser(std::move(onFileSelected), std::move(onCancel));
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
