#include "Job.h"
#include <stdio.h>
#include <gtk/gtk.h>

Job::Job() {
	XOJ_INIT_TYPE(Job);

	this->afterRunId = 0;

	this->refCount = 1;
	this->refMutex = g_mutex_new();
}

Job::~Job() {
	XOJ_CHECK_TYPE(Job);

	g_mutex_free(this->refMutex);
	this->refMutex = NULL;

	XOJ_RELEASE_TYPE(Job);
}

void Job::unref() {
	XOJ_CHECK_TYPE(Job);

	g_mutex_lock(this->refMutex);
	this->refCount--;

	if (this->refCount == 0) {
		g_mutex_unlock(this->refMutex);
		delete this;
	} else {
		g_mutex_unlock(this->refMutex);
	}
}

void Job::ref() {
	XOJ_CHECK_TYPE(Job);

	g_mutex_lock(this->refMutex);
	this->refCount++;
	g_mutex_unlock(this->refMutex);
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
	job->unref();
	return false; // do not call again
}

void Job::callAfterRun() {
	XOJ_CHECK_TYPE(Job);

	if(this->afterRunId) {
		return;
	}

	this->ref();
	this->afterRunId = g_idle_add((GSourceFunc) Job::callAfterCallback, this);
}

void Job::afterRun() {
}
