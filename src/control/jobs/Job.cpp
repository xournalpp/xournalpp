#include "Job.h"
#include <stdio.h>
#include <gtk/gtk.h>

Job::Job() {
	XOJ_INIT_TYPE(Job);

	this->afterRunId = 0;
}

Job::~Job() {
	XOJ_RELEASE_TYPE(Job);
}

void Job::free() {
	XOJ_CHECK_TYPE(Job);

	if (!this->afterRunId) {
		delete this;
	}
}

void Job::execute() {
	XOJ_CHECK_TYPE(Job);

	this->run();
}

void * Job::getSource() {
	XOJ_CHECK_TYPE(Job);

	return NULL;
}

bool Job::callAfterCallback(Job * job) {
	XOJ_CHECK_TYPE_OBJ(job, Job);

	gdk_threads_enter();
	job->afterRun();
	gdk_threads_leave();

	job->afterRunId = 0;
	job->free();
	return false; // do not call again
}

void Job::callAfterRun() {
	XOJ_CHECK_TYPE(Job);

	if(this->afterRunId) {
		return;
	}
	this->afterRunId = g_idle_add((GSourceFunc) Job::callAfterCallback, this);
}

void Job::afterRun() {
}
