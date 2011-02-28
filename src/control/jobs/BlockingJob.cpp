#include "BlockingJob.h"
#include "../Control.h"

BlockingJob::BlockingJob(Control * control, const char * name) {
	this->control = control;
	CHECK_MEMORY(control);

	control->block(name);
}

BlockingJob::~BlockingJob() {
	this->control = NULL;
}

void BlockingJob::execute() {
	CHECK_MEMORY(this->control);

	this->run();

	g_idle_add((GSourceFunc) finished, this->control);
}

bool BlockingJob::finished(Control * control) {
	gdk_threads_enter();
	control->unblock();
	gdk_threads_leave();

	// do not call again
	return false;
}

JobType BlockingJob::getType() {
	return JOB_TYPE_BLOCKING;
}
