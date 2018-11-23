#include "Job.h"

#include <gtk/gtk.h>

Job::Job()
{
	XOJ_INIT_TYPE(Job);

	this->afterRunId = 0;

	this->refCount = 1;
	g_mutex_init(&this->refMutex);
}

Job::~Job()
{
	XOJ_CHECK_TYPE(Job);

	XOJ_RELEASE_TYPE(Job);
}

void Job::unref()
{
	XOJ_CHECK_TYPE(Job);

	g_mutex_lock(&this->refMutex);
	this->refCount--;

	if (this->refCount == 0)
	{
		g_mutex_unlock(&this->refMutex);
		delete this;
	}
	else
	{
		g_mutex_unlock(&this->refMutex);
	}
}

void Job::ref()
{
	XOJ_CHECK_TYPE(Job);

	g_mutex_lock(&this->refMutex);
	this->refCount++;
	g_mutex_unlock(&this->refMutex);
}

void Job::deleteJob()
{
	if (this->afterRunId)
	{
		g_source_remove(this->afterRunId);
		this->unref();
	}
}

void Job::execute()
{
	XOJ_CHECK_TYPE(Job);

	this->run();
}

void* Job::getSource()
{
	XOJ_CHECK_TYPE(Job);

	return NULL;
}

bool Job::callAfterCallback(Job* job)
{
	XOJ_CHECK_TYPE_OBJ(job, Job);

	job->afterRun();

	job->afterRunId = 0;
	job->unref();
	return false; // do not call again
}

void Job::callAfterRun()
{
	XOJ_CHECK_TYPE(Job);

	if (this->afterRunId)
	{
		return;
	}

	this->ref();

	this->afterRunId = gdk_threads_add_idle((GSourceFunc) Job::callAfterCallback, this);
}

/**
 * After run will be called from UI Thread after the Job is finished
 *
 * All UI Stuff should happen here
 */
void Job::afterRun()
{
}
