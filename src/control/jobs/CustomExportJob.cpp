#include "CustomExportJob.h"

#include "control/Control.h"
#include "gui/dialog/ExportDialog.h"

#include <i18n.h>


CustomExportJob::CustomExportJob(Control* control)
 : BaseExportJob(control, _("Custom Export"))
{
	XOJ_INIT_TYPE(CustomExportJob);
}

CustomExportJob::~CustomExportJob()
{
	XOJ_RELEASE_TYPE(CustomExportJob);
}

void CustomExportJob::addFilterToDialog()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	addFileFilterToDialog(_C("PDF files"), "*.pdf");
	addFileFilterToDialog(_C("PNG graphics"), "*.png");
}

void CustomExportJob::addExtensionToFilePath()
{
	XOJ_CHECK_TYPE(CustomExportJob);

	GtkFileFilter* selectedFilter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));

    if (filename.extension() != ".pdf")
    {
    	this->filename += ".pdf";
    }
}

bool CustomExportJob::showFilechooser()
{
	if (!BaseExportJob::showFilechooser())
	{
		return false;
	}

	Document* doc = control->getDocument();

	doc->lock();
	int count = doc->getPageCount();
	ExportDialog* dlg = new ExportDialog(control->getGladeSearchPath(), control->getSettings(), count, control->getCurrentPageNo());
	dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));

//	PageRangeVector selected = dlg->getRange();
//
//	if (!selected.empty())
//	{
//		/*
//		ExportJob* job = new ExportJob(control, selected, dlg->getFormatType(), dlg->getPngDpi(), dlg->getFilePath());
//		control->getScheduler()->addJob(job, JOB_PRIORITY_NONE);
//		job->unref();
//		*/
//	}

	delete dlg;
	doc->unlock();
	return true;
}


void CustomExportJob::run(bool noThreads)
{
	XOJ_CHECK_TYPE(CustomExportJob);

//	SynchronizedProgressListener pglistener(this->control);
//
//	Document* doc = control->getDocument();
//	doc->lock();
//	PdfExport pdf(doc, &pglistener);
//	doc->unlock();
//
//	if (!pdf.createPdf(this->filename))
//	{
//		if (control->getWindow())
//		{
//			callAfterRun();
//		}
//		else
//		{
//			g_error("%s%s", "Create pdf failed: ", pdf.getLastError().c_str());
//		}
//	}
}
