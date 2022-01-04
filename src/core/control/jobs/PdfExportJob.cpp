#include "PdfExportJob.h"

#include "control/Control.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "util/i18n.h"

PdfExportJob::PdfExportJob(Control* control): BaseExportJob(control, _("PDF Export")) {}

PdfExportJob::~PdfExportJob() = default;

void PdfExportJob::addFilterToDialog() { addFileFilterToDialog(_("PDF files"), "*.pdf"); }

auto PdfExportJob::testAndSetFilepath(fs::path file) -> bool {
    if (!BaseExportJob::testAndSetFilepath(std::move(file))) {
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
    std::unique_ptr<XojPdfExport> pdfe = XojPdfExportFactory::createExport(doc, control);
    doc->unlock();

    if (!pdfe->createPdf(this->filepath, false)) {
        this->errorMsg = pdfe->getLastError();
        if (control->getWindow()) {
            callAfterRun();
        }
    }
}
