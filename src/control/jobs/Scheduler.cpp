#include "Scheduler.h"
#include <config-debug.h>

#include <inttypes.h>

#ifdef DEBUG_SHEDULER
#define SDEBUG g_message
#else
#define SDEBUG(msg, ...)
#endif

Scheduler::Scheduler()
{
	this->name = "Scheduler";

	// Thread
	g_cond_init(&this->jobQueueCond);

	g_mutex_init(&this->jobQueueMutex);
	g_mutex_init(&this->jobRunningMutex);
	g_mutex_init(&this->schedulerMutex);
	g_mutex_init(&this->blockRenderMutex);

	// Queue
	GQueue init = G_QUEUE_INIT;
	this->queueUrgent = init;
	this->queueHigh = init;
	this->queueLow = init;
	this->queueNone = init;
	this->jobQueue[JOB_PRIORITY_URGENT] = &this->queueUrgent;
	this->jobQueue[JOB_PRIORITY_HIGH] = &this->queueHigh;
	this->jobQueue[JOB_PRIORITY_LOW] = &this->queueLow;
	this->jobQueue[JOB_PRIORITY_NONE] = &this->queueNone;
}

Scheduler::~Scheduler()
{
	SDEBUG("Destroy scheduler");

	if (this->jobRenderThreadTimerId)
	{
		g_source_remove(this->jobRenderThreadTimerId);
		this->jobRenderThreadTimerId = 0;
	}

	stop();

	Job * job = nullptr;
	while ((job = getNextJobUnlocked()) != nullptr)
	{
		job->unref();
	}

	if (this->blockRenderZoomTime)
	{
		g_free(this->blockRenderZoomTime);
	}
}

void Scheduler::start()
{
	SDEBUG("Starting scheduler");
	g_return_if_fail(this->thread == nullptr);

	this->thread = g_thread_new(name.c_str(), (GThreadFunc) jobThreadCallback, this);
}

void Scheduler::stop()
{
	SDEBUG("Stopping scheduler");

	if (!this->threadRunning)
	{
		return;
	}
	this->threadRunning = false;
	g_cond_broadcast(&this->jobQueueCond);

	if (this->thread)
	{
		g_thread_join(this->thread);
	}
}

void Scheduler::addJob(Job* job, JobPriority priority)
{
	SDEBUG("Adding job...");

	g_mutex_lock(&this->jobQueueMutex);

	job->ref();
	g_queue_push_tail(this->jobQueue[priority], job);
	g_cond_broadcast(&this->jobQueueCond);

	SDEBUG("add job: %" PRId64, (uint64_t) job);

	g_mutex_unlock(&this->jobQueueMutex);
}

Job* Scheduler::getNextJobUnlocked(bool onlyNotRender, bool* hasRenderJobs)
{
	Job* job = nullptr;

	for (int i = JOB_PRIORITY_URGENT; i < JOB_N_PRIORITIES; i++)
	{
		if (onlyNotRender)
		{
			for (GList* l = this->jobQueue[i]->head; l != nullptr; l = l->next)
			{
				job = (Job*) l->data;

				if (job->getType() != JOB_TYPE_RENDER)
				{
					g_queue_delete_link(this->jobQueue[i], l);
					return job;
				}
				else if (hasRenderJobs)
				{
					*hasRenderJobs = true;
				}
			}
		}
		else
		{
			job = (Job*) g_queue_pop_head(this->jobQueue[i]);
			if (job)
			{
				return job;
			}
		}
	}

	return nullptr;
}

/**
 * Locks the complete scheduler
 */
void Scheduler::lock()
{
	g_mutex_lock(&this->schedulerMutex);
}

/**
 * Unlocks the complete scheduler
 */
void Scheduler::unlock()
{
	g_mutex_unlock(&this->schedulerMutex);
}

#define ZOOM_WAIT_US_TIMEOUT 300000 // 0.3s

void Scheduler::blockRerenderZoom()
{
	g_mutex_lock(&this->blockRenderMutex);

	if (this->blockRenderZoomTime == nullptr)
	{
		this->blockRenderZoomTime = g_new(GTimeVal, 1);
	}

	g_get_current_time(this->blockRenderZoomTime);
	g_time_val_add(this->blockRenderZoomTime, ZOOM_WAIT_US_TIMEOUT);

	g_mutex_unlock(&this->blockRenderMutex);
}

void Scheduler::unblockRerenderZoom()
{
	g_mutex_lock(&this->blockRenderMutex);

	g_free(this->blockRenderZoomTime);
	this->blockRenderZoomTime = nullptr;
	if (this->jobRenderThreadTimerId)
	{
		g_source_remove(this->jobRenderThreadTimerId);
		this->jobRenderThreadTimerId = 0;
	}

	g_mutex_unlock(&this->blockRenderMutex);

	g_cond_broadcast(&this->jobQueueCond);
}

/**
 * g_time_val_diff:
 * @t1: time value t1
 * @t2: time value t2
 *
 * Calculates the time difference between t1 and t2 in milliseconds.
 * The result is positive if t1 is later than t2.
 *
 * Returns:
 * Time difference in microseconds
 */
glong g_time_val_diff(GTimeVal* t1, GTimeVal* t2)
{
	g_assert(t1);
	g_assert(t2);
	return ((t1->tv_sec - t2->tv_sec) * G_USEC_PER_SEC + (t1->tv_usec - t2->tv_usec)) / 1000;
}

/**
 * If the Scheduler is blocking because we are zooming and there are only render jobs
 * we need to wakeup it later
 */
bool Scheduler::jobRenderThreadTimer(Scheduler* scheduler)
{
	scheduler->jobRenderThreadTimerId = 0;

	g_mutex_lock(&scheduler->blockRenderMutex);
	g_free(scheduler->blockRenderZoomTime);
	scheduler->blockRenderZoomTime = nullptr;
	g_mutex_unlock(&scheduler->blockRenderMutex);

	g_cond_broadcast(&scheduler->jobQueueCond);

	return false;
}

gpointer Scheduler::jobThreadCallback(Scheduler* scheduler)
{
	while (scheduler->threadRunning)
	{
		// lock the whole scheduler
		g_mutex_lock(&scheduler->schedulerMutex);

		g_mutex_lock(&scheduler->blockRenderMutex);
		bool onlyNoneRenderJobs = false;
		glong diff = 1000;
		if (scheduler->blockRenderZoomTime)
		{
			GTimeVal time;
			g_get_current_time(&time);

			diff = g_time_val_diff(scheduler->blockRenderZoomTime, &time);
			if (diff <= 0)
			{
				g_free(scheduler->blockRenderZoomTime);
				scheduler->blockRenderZoomTime = nullptr;
			}
			else
			{
				onlyNoneRenderJobs = true;
			}
		}
		g_mutex_unlock(&scheduler->blockRenderMutex);

		g_mutex_lock(&scheduler->jobQueueMutex);
		bool hasOnlyRenderJobs = false;
		Job* job = scheduler->getNextJobUnlocked(onlyNoneRenderJobs, &hasOnlyRenderJobs);
		if (job != nullptr)
		{
			hasOnlyRenderJobs = false;
		}

		SDEBUG("get job: %" PRId64, (uint64_t) job);

		if (job == nullptr)
		{
			// unlock the whole scheduler
			g_mutex_unlock(&scheduler->schedulerMutex);

			if (hasOnlyRenderJobs)
			{
				if (scheduler->jobRenderThreadTimerId)
				{
					g_source_remove(scheduler->jobRenderThreadTimerId);
				}
				scheduler->jobRenderThreadTimerId = g_timeout_add(diff, (GSourceFunc) jobRenderThreadTimer, scheduler);
			}

			g_cond_wait(&scheduler->jobQueueCond, &scheduler->jobQueueMutex);
			g_mutex_unlock(&scheduler->jobQueueMutex);

			continue;
		}

		SDEBUG("do job: %" PRId64, (uint64_t) job);

		g_mutex_unlock(&scheduler->jobQueueMutex);

		g_mutex_lock(&scheduler->jobRunningMutex);

		job->execute();

		job->unref();
		g_mutex_unlock(&scheduler->jobRunningMutex);

		// unlock the whole scheduler
		g_mutex_unlock(&scheduler->schedulerMutex);

		SDEBUG("next");
	}

	SDEBUG("finished");

	return nullptr;
}
