#include "BlockingJob.h"

#include "control/Control.h"
#include "control/xojfile/SaveHandler.h"

BlockingJob::BlockingJob(Control* control, string name)
 : control(control)
{
	control->block(name);
}

BlockingJob::~BlockingJob()
{
	this->control = nullptr;
}

void BlockingJob::execute()
{
	this->run();

	g_idle_add((GSourceFunc) finished, this->control);
}

bool BlockingJob::finished(Control* control)
{
	// "this" is not needed, "control" is in
	// the closure, therefore no sync needed
	Util::execInUiThread([=]() {
		control->unblock();
	});

	// do not call again
	return false;
}

JobType BlockingJob::getType()
{
	return JOB_TYPE_BLOCKING;
}
