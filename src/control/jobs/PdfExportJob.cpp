#include "PdfExportJob.h"

#include "control/Control.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"

#include "i18n.h"

PdfExportJob::PdfExportJob(Control* control): BaseExportJob(control, _("PDF Export")) {}

PdfExportJob::~PdfExportJob() = default;

void PdfExportJob::addFilterToDialog() { addFileFilterToDialog(_("PDF files"), "*.pdf"); }

auto PdfExportJob::isUriValid(string& uri) -> bool {
    if (!BaseExportJob::isUriValid(uri)) {
        return false;
    }

    // Remove any pre-existing extension and adds .pdf
    Util::clearExtensions(filepath, ".pdf");
    filepath += ".pdf";

    return checkOverwriteBackgroundPDF(filepath);
}


void PdfExportJob::run() {
    Document* doc = control->getDocument();

    doc->lock();
    XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, control);
    doc->unlock();

    if (!pdfe->createPdf(this->filepath)) {
        if (control->getWindow()) {
            callAfterRun();
        } else {
            this->errorMsg = pdfe->getLastError();
        }
    }

    delete pdfe;
}
