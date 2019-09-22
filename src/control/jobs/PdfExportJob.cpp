#include "PdfExportJob.h"
#include "control/Control.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"

#include <i18n.h>

PdfExportJob::PdfExportJob(Control* control)
 : BaseExportJob(control, _("PDF Export"))
{
}

PdfExportJob::~PdfExportJob()
{
}

void PdfExportJob::addFilterToDialog()
{
	addFileFilterToDialog(_("PDF files"), "*.pdf");
}

bool PdfExportJob::isUriValid(string& uri)
{
	if (!BaseExportJob::isUriValid(uri))
	{
		return false;
	}

	// Remove any pre-existing extension and adds .pdf
	filename.clearExtensions(".pdf");
	filename += ".pdf";
	
	return checkOverwriteBackgroundPDF(filename);
}


void PdfExportJob::run()
{
	Document* doc = control->getDocument();

	doc->lock();
	XojPdfExport* pdfe = XojPdfExportFactory::createExport(doc, control);
	doc->unlock();

	if (!pdfe->createPdf(this->filename))
	{
		if (control->getWindow())
		{
			callAfterRun();
		}
		else
		{
			this->errorMsg = pdfe->getLastError();
		}
	}

	delete pdfe;
}

