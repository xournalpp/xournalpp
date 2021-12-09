#include "PdfExportJob.h"

#include "control/Control.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"
#include "util/XojMsgBox.h"
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
    printf("now exporting\n");
    Document* doc = control->getDocument();

    doc->lock();
    XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, control);
    doc->unlock();

    if (!pdfe->createPdf(this->filepath, false)) {
        this->errorMsg = pdfe->getLastError();
        if (control->getWindow()) {
            callAfterRun();
        }
    } else {
        callAfterRun();
    }

    delete pdfe;
}

void PdfExportJob::afterRun() {
    /* if (!this->errorMsg.empty()) { */
    /*     XojMsgBox::showErrorToUser(control->getGtkWindow(), this->errorMsg); */
    /* } */
    // TODO: what is with custom export job?
    printf("after run pdfexportjob\n");
    if (this->overwriteBackground) {
        if (this->makeBackgroundBackup) {
            /* We previously changed the background pdf to the backup. Keep the document open as is but fake a saving
             * action. */
            this->control->undoRedoFakeSaved();
        } else {
            /* We previously saved the background pdf temporarily and changed the background to it. */
            fs::path tmp = this->control->getDocument()->getPdfFilepath();
            /* Discard old changes and load newly created pdf file. */
            this->control->discardDocument();  // TODO: calling discardDocument and newFile here should not
            this->control->newFile();          //       be needed but it doesn't work without both of them
            this->control->annotatePdf(this->filepath, true, true, true);
            /* Remove the tmp background pdf. */
            remove(tmp);
        }
    }
}
