#include "PdfExportJob.h"

#include <memory>   // for unique_ptr, allocator
#include <string>   // for string
#include <utility>  // for move

#include "control/Control.h"               // for Control
#include "control/jobs/BaseExportJob.h"    // for BaseExportJob
#include "model/Document.h"                // for Document
#include "pdf/base/XojPdfExport.h"         // for XojPdfExport
#include "pdf/base/XojPdfExportFactory.h"  // for XojPdfExportFactory
#include "util/PathUtil.h"                 // for clearExtensions
#include "util/i18n.h"                     // for _

PdfExportJob::PdfExportJob(Control* control): BaseExportJob(control, _("PDF Export")) {}

PdfExportJob::~PdfExportJob() = default;

void PdfExportJob::addFilterToDialog() {
     addFileFilterToDialog(_("PDF files"), ".pdf");
}

auto PdfExportJob::testAndSetFilepath(const fs::path& file) -> bool {
    if (!BaseExportJob::testAndSetFilepath(file)) {
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
