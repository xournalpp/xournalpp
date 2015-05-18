#include "Control.h"
#include "ExportHandler.h"

#include "gui/dialog/ExportDialog.h"
#include "jobs/ExportJob.h"
#include "model/Document.h"

#include <PageRange.h>

ExportHandler::ExportHandler()
{
	XOJ_INIT_TYPE(ExportHandler);
}

ExportHandler::~ExportHandler()
{
	XOJ_RELEASE_TYPE(ExportHandler);
}

void ExportHandler::runExportWithDialog(GladeSearchpath* gladeSearchPath,
										Settings* settings, Document* doc,
										Control* control, int current)
{
	XOJ_CHECK_TYPE(ExportHandler);

	doc->lock();
	int count = doc->getPageCount();
	ExportDialog* dlg = new ExportDialog(gladeSearchPath, settings, count, current);
	dlg->show(GTK_WINDOW(control->getWindow()->getWindow()));

	PageRangeVector selected = dlg->getRange();

	if (!selected.empty())
	{
		ExportJob* job = new ExportJob(control, selected, dlg->getFormatType(), dlg->getPngDpi(), dlg->getFilePath());
		control->getScheduler()->addJob(job, JOB_PRIORITY_NONE);
		job->unref();
	}

	delete dlg;
	doc->unlock();
}
