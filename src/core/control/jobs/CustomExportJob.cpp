#include "CustomExportJob.h"

#include <memory>   // for unique_ptr
#include <utility>  // for move

#include "control/Control.h"                   // for Control
#include "control/jobs/BaseExportJob.h"        // for BaseExportJob::ExportType
#include "control/jobs/ExportParameters.h"     // for ExportParameters
#include "control/xojfile/XojExportHandler.h"  // for XojExportHandler
#include "model/Document.h"                    // for Document
#include "pdf/base/XojPdfExport.h"             // for XojPdfExport
#include "pdf/base/XojPdfExportFactory.h"      // for XojPdfExportFactory
#include "util/XojMsgBox.h"                    // for XojMsgBox
#include "util/i18n.h"                         // for _, FS, _F

#include "ImageExport.h"  // for ImageExport, EXPORT_GR...
#include "SaveJob.h"      // for SaveJob


CustomExportJob::CustomExportJob(Control* control, fs::path output, std::unique_ptr<ExportParameters> params):
        BaseExportJob(control, _("Custom Export")), parameters(std::move(params)) {
    this->filepath = std::move(output);
}

CustomExportJob::~CustomExportJob() = default;

/**
 * Create one Graphics file per page
 */
void CustomExportJob::exportGraphics() {
    ImageExport imgExport(control->getDocument(), filepath, parameters->format, parameters->backgroundType,
                          parameters->pageRanges);
    if (parameters->format == EXPORT_GRAPHICS_PNG) {
        imgExport.setQualityParameter(parameters->qualityParameter);
    }
    imgExport.exportGraphics(control);
    errorMsg = imgExport.getLastErrorMsg();
}

void CustomExportJob::run() {
    if (parameters->format == EXPORT_XOJ) {
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
    } else if (parameters->format == EXPORT_GRAPHICS_PDF) {
        // don't lock the page here for the whole flow, else we get a dead lock...
        // the ui is blocked, so there should be no changes...
        Document* doc = control->getDocument();

        std::unique_ptr<XojPdfExport> pdfe =
                XojPdfExportFactory::createExport(doc, control, parameters->pdfExportBackend);

        pdfe->setExportBackground(parameters->backgroundType);

        if (!pdfe->createPdf(this->filepath, parameters->pageRanges, parameters->progressiveMode)) {
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
