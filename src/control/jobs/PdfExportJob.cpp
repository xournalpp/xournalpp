#include "PdfExportJob.h"
#include "control/Control.h"
#include "pdf/base/XojPdfExport.h"
#include "pdf/base/XojPdfExportFactory.h"

#include <i18n.h>
#include <iostream>


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
	addFileFilterToDialog(_C("PDF Without paper style"), "*.pdf");
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


	std::cout<<"PdfExportJob::isUriValid\n";
	string filterName = BaseExportJob::getFilterName();
	std::cout<<"FilterName: "<<filterName<<"\n";

	return true;
}

void PdfExportJob::resetBackgroundType(Document* doc, PageType* pt, ResetActionType action)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	size_t count = doc->getPageCount();

	if (action == ACTION_RESET)
	{
		/** apply "plain" paper style to all pages before export */
		for (int i=0; i<count ; i++)
		{
			pt[i] = doc->getPage(i)->getBackgroundType();
			doc->getPage(i)->setBackgroundType(PageType("plain"));	
		}
	}

	if (action == ACTION_RESTORE)
	{
		/** restore each page to its original style */
		for (int i=0; i<count ; i++)
		{
			doc->getPage(i)->setBackgroundType(pt[i]);	
		}
	}
	
}

void PdfExportJob::run()
{
	XOJ_CHECK_TYPE(PdfExportJob);

	Document* doc = control->getDocument();

	size_t count = doc->getPageCount();
	PageType pt[count];

	resetBackgroundType(doc, pt, ACTION_RESET);

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

	resetBackgroundType(doc, pt, ACTION_RESTORE);
}

