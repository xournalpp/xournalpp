#include "CustomExportJob.h"

#include <memory>   // for unique_ptr
#include <utility>  // for move, pair

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
#include "util/XojMsgBox.h"                    // for XojMsgBox
#include "util/i18n.h"                         // for _, FS, _F

#include "ImageExport.h"  // for ImageExport, EXPORT_GR...
#include "SaveJob.h"      // for SaveJob
#include "XournalScheduler.h"


CustomExportJob::CustomExportJob(Control* control): BaseExportJob(control, _("Custom Export")) {
    // Supported filters
    filters.insert({_("PDF files"), ExportType(".pdf")});
    filters.insert({_("PNG graphics"), ExportType(".png")});
    filters.insert({_("SVG graphics"), ExportType(".svg")});
    filters.insert({_("Xournal (Compatibility)"), ExportType(".xoj")});
}

CustomExportJob::~CustomExportJob() = default;

void CustomExportJob::addFilterToDialog() {
    // Runs on every filter inside the filters map
    for (auto& filter: filters) {
        addFileFilterToDialog(filter.first, "*" + filter.second.extension);  // Adds * for the pattern
    }
}

auto CustomExportJob::testAndSetFilepath(const fs::path& file) -> bool {
    if (!BaseExportJob::testAndSetFilepath(std::move(file))) {
        return false;
    }

    // Extract the file filter selected
    this->chosenFilterName = BaseExportJob::getFilterName();
    const auto& chosenFilter = filters.at(this->chosenFilterName);

    // Remove any pre-existing extension and adds the chosen one
    Util::clearExtensions(filepath, chosenFilter.extension);
    filepath += chosenFilter.extension;

    return checkOverwriteBackgroundPDF(filepath);
}

auto CustomExportJob::showFilechooser() -> bool {
    if (!BaseExportJob::showFilechooser()) {
        control->unblock();
        return false;
    }

    if (filepath.extension() == ".xoj") {
        exportTypeXoj = true;
        control->getScheduler()->addJob(this, JOB_PRIORITY_NONE);
        return true;
    }

    if (auto ext = filepath.extension(); ext == ".pdf") {
        format = EXPORT_GRAPHICS_PDF;
    } else if (ext == ".svg") {
        format = EXPORT_GRAPHICS_SVG;
    } else if (ext == ".png") {
        format = EXPORT_GRAPHICS_PNG;
    } else {
        g_warning("Unknown extension");
    }

    this->ref();
    xoj::popup::PopupWindowWrapper<xoj::popup::ExportDialog> popup(
            control->getGladeSearchPath(), format, control->getCurrentPageNo() + 1,
            control->getDocument()->getPageCount(), [job = this](const xoj::popup::ExportDialog& dialog) {
                if (dialog.isConfirmed()) {
                    job->exportRange = dialog.getRange();
                    job->progressiveMode = dialog.progressiveModeSelected();
                    job->exportBackground = dialog.getBackgroundType();

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
    popup.show(GTK_WINDOW(control->getWindow()->getWindow()));

    return true;
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
        doc->lock();
        h.prepareSave(doc, filepath);
        h.saveTo(filepath, this->control);
        doc->unlock();

        if (!h.getErrorMessage().empty()) {
            this->lastError = FS(_F("Save file error: {1}") % h.getErrorMessage());

            callAfterRun();
        }
    } else if (format == EXPORT_GRAPHICS_PDF) {
        // don't lock the page here for the whole flow, else we get a dead lock...
        // the ui is blocked, so there should be no changes...
        Document* doc = control->getDocument();

        std::unique_ptr<XojPdfExport> pdfe = XojPdfExportFactory::createExport(doc, control);

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
