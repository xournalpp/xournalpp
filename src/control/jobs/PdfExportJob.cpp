#include "PdfExportJob.h"
#include "../../pdf/PdfExport.h"
#include "../Control.h"
#include "SynchronizedProgressListener.h"

PdfExportJob::PdfExportJob(Control * control) :
	BlockingJob(control, _("PDF Export")) {
}

PdfExportJob::~PdfExportJob() {
}

void PdfExportJob::run() {
	SynchronizedProgressListener pglistener(this->control);

	Document * doc = control->getDocument();
	CHECK_MEMORY(doc);
	PdfExport pdf(doc, &pglistener);

	// TODO: debug
	if (!pdf.createPdf("file:///home/andreas/tmp/pdf/pdffile.pdf")) {
		printf("create pdf failed\n");
		printf("error: %s\n", pdf.getLastError().c_str());
	}
}
