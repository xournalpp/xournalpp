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
#include "util/PathUtil.h"                     // for clearExtensions
#include "util/XojMsgBox.h"                    // for XojMsgBox
#include "util/i18n.h"                         // for _, FS, _F

#include "ImageExport.h"  // for ImageExport, EXPORT_GR...
#include "SaveJob.h"      // for SaveJob


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
        return false;
    }

    if (filepath.extension() == ".xoj") {
        exportTypeXoj = true;
        return true;
    }

    Document* doc = control->getDocument();
    doc->lock();
    ExportDialog dlg(control->getGladeSearchPath());
    if (filepath.extension() == ".pdf") {
        dlg.showProgressiveMode();
        format = EXPORT_GRAPHICS_PDF;
    } else if (filepath.extension() == ".svg") {
        dlg.removeQualitySetting();
        format = EXPORT_GRAPHICS_SVG;
    } else if (filepath.extension() == ".png") {
        format = EXPORT_GRAPHICS_PNG;
    }

    dlg.initPages(control->getCurrentPageNo() + 1, doc->getPageCount());

    dlg.show(GTK_WINDOW(control->getWindow()->getWindow()));

    if (!dlg.isConfirmed()) {
        doc->unlock();
        return false;
    }

    exportRange = dlg.getRange();
    progressiveMode = dlg.progressiveMode();
    exportBackground = dlg.getBackgroundType();

    if (format == EXPORT_GRAPHICS_PNG) {
        pngQualityParameter = dlg.getPngQualityParameter();
    }

    doc->unlock();
    return true;
}

/**
 * Create one Graphics file per page
 */
void CustomExportJob::exportGraphics() {
    ImageExport imgExport{control->getDocument(), filepath, format};
    imgExport.setExportBackground(exportBackground);
    imgExport.setExportRange(exportRange);
    imgExport.setProgressListener(control);

    if (format == EXPORT_GRAPHICS_PNG) {
        imgExport.setQualityParameter(pngQualityParameter);
    }
    imgExport.exportDocument();

    if (imgExport.getLastErrorMsg()) {
        errorMsg = imgExport.getLastErrorMsg().value();
    }
}

void CustomExportJob::run() {
    if (exportTypeXoj) {
        SaveJob::updatePreview(control);
        Document* doc = this->control->getDocument();

        XojExportHandler h;
        doc->lock();
        h.prepareSave(doc);
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

        XojPdfExport pdfe{doc, filepath};
        pdfe.setExportBackground(exportBackground);
        pdfe.setProgressListener(control);
        pdfe.setExportRange(exportRange);
        pdfe.setProgressiveMode(progressiveMode);

        pdfe.exportDocument();
        if (pdfe.getLastErrorMsg()) {
            errorMsg = pdfe.getLastErrorMsg().value();
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
