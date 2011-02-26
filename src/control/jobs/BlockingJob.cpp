#include "BlockingJob.h"
#include "../Control.h"

BlockingJob::BlockingJob(Control * control, const char * name) {
	this->control = control;
	CHECK_MEMORY(control);

	// Disable all gui Control, to get full control over the application
	control->getWindow()->setControlTmpDisabled(true);
	control->getCursor()->setCursorBusy(true);
	control->setSidebarTmpDisabled(true);

	MainWindow * win = control->getWindow();
	this->statusbar = win->get("statusbar");
	this->lbState = GTK_LABEL(win->get("lbState"));
	this->pgState = GTK_PROGRESS_BAR(win->get("pgState"));

	gtk_label_set_text(this->lbState, name);
	gtk_widget_show(this->statusbar);

	this->maxState = 100;
}

BlockingJob::~BlockingJob() {
	this->control = NULL;
}

void BlockingJob::execute() {
	CHECK_MEMORY(this->control);

	this->run();

	gdk_threads_enter();
	this->finished();
	gdk_threads_leave();
}

void BlockingJob::finished() {
	control->getWindow()->setControlTmpDisabled(false);
	control->getCursor()->setCursorBusy(false);
	control->setSidebarTmpDisabled(false);

	gtk_widget_hide(this->statusbar);
}

JobType BlockingJob::getType() {
	return JOB_TYPE_BLOCKING;
}

void BlockingJob::setMaximumState(int max) {
	this->maxState = max;
}

void BlockingJob::setCurrentState(int state) {
	gtk_progress_bar_set_fraction(this->pgState, (double) state / this->maxState);
}

