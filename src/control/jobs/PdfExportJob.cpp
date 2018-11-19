#include "PdfExportJob.h"

#include "control/Control.h"
#include "pdf/popplerdirect/PdfExport.h"
#include "SynchronizedProgressListener.h"

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

void PdfExportJob::addExtensionToFilePath()
{
	XOJ_CHECK_TYPE(PdfExportJob);

    if (filename.extension() != ".pdf")
    {
    	this->filename += ".pdf";
    }
}

void PdfExportJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(PdfExportJob);

	SynchronizedProgressListener pglistener(this->control);

	Document* doc = control->getDocument();
	doc->lock();
	PdfExport pdf(doc, &pglistener);
	doc->unlock();

	if (!pdf.createPdf(this->filename))
	{
		if (control->getWindow())
		{
			callAfterRun();
		}
		else
		{
			g_error("%s%s", "Create pdf failed: ", pdf.getLastError().c_str());
		}
	}
}

