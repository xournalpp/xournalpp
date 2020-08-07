#include "CustomExportJob.h"

#include <config-features.h>

#include "control/Control.h"
#include "control/xojfile/XojExportHandler.h"
#include "gui/dialog/ExportDialog.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "view/PdfView.h"

#include "ImageExport.h"
#include "PathUtil.h"
#include "SaveJob.h"
#include "XojMsgBox.h"
#include "i18n.h"


CustomExportJob::CustomExportJob(Control* control): BaseExportJob(control, _("Custom Export")) {
    // Supported filters
    filters[_("PDF files")] = new ExportType(".pdf", false);
    filters[_("PDF with plain background")] = new ExportType(".pdf", true);
    filters[_("PNG graphics")] = new ExportType(".png", false);
    filters[_("PNG with transparent background")] = new ExportType(".png", true);
    filters[_("SVG graphics")] = new ExportType(".svg", false);
    filters[_("SVG with transparent background")] = new ExportType(".svg", true);
    filters[_("Xournal (Compatibility)")] = new ExportType(".xoj", false);
}

CustomExportJob::~CustomExportJob() {
    for (PageRangeEntry* e: exportRange) {
        delete e;
    }
    exportRange.clear();

    for (auto& filter: filters) {
        delete filter.second;
    }
}

void CustomExportJob::addFilterToDialog() {
    // Runs on every filter inside the filters map
    for (auto& filter: filters) {
        addFileFilterToDialog(filter.first, "*" + filter.second->extension);  // Adds * for the pattern
    }
}

auto CustomExportJob::isUriValid(string& uri) -> bool {
    if (!BaseExportJob::isUriValid(uri)) {
        return false;
    }

    // Extract the file filter selected
    this->chosenFilterName = BaseExportJob::getFilterName();

    // Remove any pre-existing extension and adds the chosen one
    Util::clearExtensions(filepath, filters[this->chosenFilterName]->extension);
    filepath += filters[this->chosenFilterName]->extension;

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
    auto* dlg = new ExportDialog(control->getGladeSearchPath());
    if (filepath.extension() == ".pdf") {
        dlg->removeDpiSelection();
        format = EXPORT_GRAPHICS_PDF;
    } else if (filepath.extension() == ".svg") {
        dlg->removeDpiSelection();
        format = EXPORT_GRAPHICS_SVG;
    } else if (filepath.extension() == ".png") {
        dlg->removeDpiSelection();
        format = EXPORT_GRAPHICS_PNG;
    }

    dlg->initPages(control->getCurrentPageNo() + 1, doc->getPageCount());

    dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));

    if (!dlg->isConfirmed()) {
        doc->unlock();
        return false;
    }

    exportRange = dlg->getRange();
    pngDpi = dlg->getPngDpi();

    delete dlg;
    doc->unlock();
    return true;
}

/**
 * Create one Graphics file per page
 */
void CustomExportJob::exportGraphics() {
    bool hideBackground = filters[this->chosenFilterName]->withoutBackground;

    ImageExport imgExport(control->getDocument(), filepath, format, hideBackground, exportRange);
    imgExport.setPngDpi(pngDpi);
    imgExport.exportGraphics(control);

    errorMsg = imgExport.getLastErrorMsg();
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

        XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, control);

        pdfe->setNoBackgroundExport(filters[this->chosenFilterName]->withoutBackground);

        if (!pdfe->createPdf(this->filepath, exportRange)) {
            this->errorMsg = pdfe->getLastError();
        }

        delete pdfe;
    } else {
        exportGraphics();
    }
}

void CustomExportJob::afterRun() {
    if (!this->lastError.empty()) {
        XojMsgBox::showErrorToUser(control->getGtkWindow(), this->lastError);
    }
}
