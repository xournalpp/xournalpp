#include "Job.h"
#include <stdio.h>
#include <gtk/gtk.h>

Job::Job() {
	this->afterRunId = 0;
}

Job::~Job() {
}

void Job::free() {
	if (!this->afterRunId) {
		delete this;
	}
}

void Job::execute() {
	this->run();
}

void * Job::getSource() {
	return NULL;
}

bool Job::callAfterCallback(Job * job) {
	gdk_threads_enter();
	job->afterRun();
	gdk_threads_leave();

	job->afterRunId = 0;
	job->free();
	return false; // do not call again
}

void Job::callAfterRun() {
	if(this->afterRunId) {
		return;
	}
	this->afterRunId = g_idle_add((GSourceFunc) Job::callAfterCallback, this);
}

void Job::afterRun() {
}
