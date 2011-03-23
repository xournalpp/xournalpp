#include "ExportHandler.h"
#include "../gui/dialog/ExportDialog.h"
#include "../util/PageRange.h"
#include "../model/Document.h"
#include "../gui/dialog/ExportDialog.h"
#include "jobs/ExportJob.h"
#include "Control.h"
// TODO: AA: type check

ExportHandler::ExportHandler() {
}

ExportHandler::~ExportHandler() {
}

void ExportHandler::runExportWithDialog(GladeSearchpath * gladeSearchPath, Settings * settings, Document * doc, Control * control, int current) {
	doc->lock();
	int count = doc->getPageCount();
	ExportDialog * dlg = new ExportDialog(gladeSearchPath, settings, count, current);
	dlg->show();

	GList * selected = dlg->getRange();

	if (selected) {
		ExportJob * job = new ExportJob(control, selected, dlg->getFormatType(), dlg->getPngDpi(), dlg->getFolder(), dlg->getFilename());
		control->getScheduler()->addJob(job, JOB_PRIORITY_NONE);
	}

	delete dlg;
	doc->unlock();
}
