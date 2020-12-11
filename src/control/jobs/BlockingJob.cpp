#include "BlockingJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"
#include "gui/XournalView.h"

BlockingJob::BlockingJob(Control* control, string name)
 : control(control)
{
	XOJ_INIT_TYPE(BlockingJob);

	control->block(name);
}

BlockingJob::~BlockingJob()
{
	XOJ_CHECK_TYPE(BlockingJob);

	this->control = NULL;

	XOJ_RELEASE_TYPE(BlockingJob);
}

void BlockingJob::execute()
{
	XOJ_CHECK_TYPE(BlockingJob);

	this->run();

	g_idle_add((GSourceFunc) finished, this->control);
}

bool BlockingJob::finished(Control* control)
{
	// "this" is not needed, "control" is in
	// the closure, therefore no sync needed
	Util::execInUiThread([=]() {
		control->unblock();
		XournalView* xournal = control->getWindow()->getXournal();
		gtk_widget_grab_focus(xournal->getWidget());
	});

	// do not call again
	return false;
}

JobType BlockingJob::getType()
{
	return JOB_TYPE_BLOCKING;
}
