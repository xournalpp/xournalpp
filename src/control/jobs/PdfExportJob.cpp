#include "PdfExportJob.h"
#include "control/Control.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"

#include <i18n.h>


PdfExportJob::PdfExportJob(Control* control)
 : BaseExportJob(control, _("PDF Export"))
{
	XOJ_INIT_TYPE(PdfExportJob);
}

PdfExportJob::~PdfExportJob()
{
	XOJ_RELEASE_TYPE(PdfExportJob);
}

void PdfExportJob::addFilterToDialog()
{
	XOJ_CHECK_TYPE(PdfExportJob);

	addFileFilterToDialog(_C("PDF files"), "*.pdf");
}

void PdfExportJob::prepareSavePath(path& path)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	if (path.extension() != ".pdf")
    {
		path += ".pdf";
    }
}

bool PdfExportJob::isUriValid(string& uri)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	if (!BaseExportJob::isUriValid(uri))
	{
		return false;
	}

	string ext = filename.extension().string();
	if (ext != ".pdf")
	{
		string msg = _C("File name needs to end with .pdf");
		Util::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}

	if (boost::iequals(filename.string(), control->getDocument()->getPdfFilename().string()))
	{
		string msg = _C("Do not overwrite the background PDF! This will cause errors!");
		Util::showErrorToUser(control->getGtkWindow(), msg);
		return false;
	}
	
	return true;
}


void PdfExportJob::run()
{
	XOJ_CHECK_TYPE(PdfExportJob);

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

