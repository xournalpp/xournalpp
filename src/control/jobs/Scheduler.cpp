#include "Scheduler.h"
#include <stdio.h>
#include "../../cfg.h"

#ifdef SHEDULER_DEBUG
#define SDEBUG(msg, ...) printf("Scheduler:: " msg, __VA_ARGS__)
#else
#define SDEBUG(msg, ...) {}
#endif

Scheduler::Scheduler() {
	XOJ_INIT_TYPE(Scheduler);

	// Thread
	this->threadRunning = true;
	this->jobQueueCond = g_cond_new();
	this->jobQueueMutex = g_mutex_new();
	this->jobRunningMutex = g_mutex_new();
	this->schedulerMutex = g_mutex_new();
	this->blockRenderMutex = g_mutex_new();

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

	this->thread = NULL;

	this->blockRenderZoomTime = NULL;

	this->jobRenderThreadTimerId = 0;
}

Scheduler::~Scheduler() {
	XOJ_CHECK_TYPE(Scheduler);

	SDEBUG("Destroy scheduler\n", 0);

	if(this->jobRenderThreadTimerId) {
		g_source_remove(this->jobRenderThreadTimerId);
		this->jobRenderThreadTimerId = 0;
	}

	stop();

	g_mutex_free(this->jobQueueMutex);
	g_mutex_free(this->jobRunningMutex);

	g_mutex_free(this->schedulerMutex);

	g_mutex_free(this->blockRenderMutex);

	g_cond_free(this->jobQueueCond);

	Job * job = NULL;
	while (job = getNextJobUnlocked()) {
		job->unref();
	}

	if (this->blockRenderZoomTime) {
		g_free(this->blockRenderZoomTime);
	}

	XOJ_RELEASE_TYPE(Scheduler);
}

void Scheduler::start() {
	g_return_if_fail(this->thread == NULL);
	this->thread = g_thread_create((GThreadFunc)jobThreadCallback, this, true, NULL);
}

void Scheduler::stop() {
	if (!this->threadRunning) {
		return;
	}
	this->threadRunning = false;
	g_cond_broadcast(this->jobQueueCond);
	g_thread_join(this->thread);
}

void Scheduler::addJob(Job * job, JobPriority priority) {
	XOJ_CHECK_TYPE(Scheduler);

	g_mutex_lock(this->jobQueueMutex);

	job->ref();
	g_queue_push_tail(this->jobQueue[priority], job);
	g_cond_broadcast(this->jobQueueCond);

	SDEBUG("add job: %ld\n", (long)job);

	g_mutex_unlock(this->jobQueueMutex);
}

Job * Scheduler::getNextJobUnlocked(bool onlyNotRender, bool * hasRenderJobs) {
	XOJ_CHECK_TYPE(Scheduler);

	Job * job = NULL;

	for (int i = JOB_PRIORITY_URGENT; i < JOB_N_PRIORITIES; i++) {
		if(onlyNotRender) {
			for(GList * l = this->jobQueue[i]->head; l != NULL; l = l->next) {
				job = (Job *) l->data;

				if(job->getType() != JOB_TYPE_RENDER) {
					g_queue_delete_link(this->jobQueue[i], l);
					return job;
				} else if(hasRenderJobs) {
					*hasRenderJobs = true;
				}
			}
		} else {
			job = (Job *) g_queue_pop_head(this->jobQueue[i]);
			if (job) {
				return job;
			}
		}
	}

	return NULL;
}

/**
 * Locks the complete scheduler
 */
void Scheduler::lock() {
	XOJ_CHECK_TYPE(Scheduler);

	g_mutex_lock(this->schedulerMutex);
}

/**
 * Unlocks the complete scheduler
 */
void Scheduler::unlock() {
	XOJ_CHECK_TYPE(Scheduler);

	g_mutex_unlock(this->schedulerMutex);
}

#define ZOOM_WAIT_US_TIMEOUT 300000 // 0.3s

void Scheduler::blockRerenderZoom() {
	XOJ_CHECK_TYPE(Scheduler);

	g_mutex_lock(this->blockRenderMutex);

	if (this->blockRenderZoomTime == NULL) {
		this->blockRenderZoomTime = g_new(GTimeVal, 1);
	}

	g_get_current_time(this->blockRenderZoomTime);
	g_time_val_add(this->blockRenderZoomTime, ZOOM_WAIT_US_TIMEOUT);

	g_mutex_unlock(this->blockRenderMutex);
}

void Scheduler::unblockRerenderZoom() {
	XOJ_CHECK_TYPE(Scheduler);

	g_mutex_lock(this->blockRenderMutex);

	g_free(this->blockRenderZoomTime);
	this->blockRenderZoomTime = NULL;
	if(this->jobRenderThreadTimerId) {
		g_source_remove(this->jobRenderThreadTimerId);
		this->jobRenderThreadTimerId = 0;
	}

	g_mutex_unlock(this->blockRenderMutex);

	g_cond_broadcast(this->jobQueueCond);
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
glong g_time_val_diff(GTimeVal * t1, GTimeVal * t2) {
	g_assert(t1);
	g_assert(t2);
	return ((t1->tv_sec - t2->tv_sec) * G_USEC_PER_SEC + (t1->tv_usec - t2->tv_usec)) / 1000;
}

/**
 * If the Scheduler is blocking because we are zooming and there are only render jobs
 * we need to wakeup it later
 */
bool Scheduler::jobRenderThreadTimer(Scheduler * scheduler) {
	XOJ_CHECK_TYPE_OBJ(scheduler, Scheduler);

	scheduler->jobRenderThreadTimerId = 0;

	g_mutex_lock(scheduler->blockRenderMutex);
	g_free(scheduler->blockRenderZoomTime);
	scheduler->blockRenderZoomTime = NULL;
	g_mutex_unlock(scheduler->blockRenderMutex);

	g_cond_broadcast(scheduler->jobQueueCond);

	return false;
}

gpointer Scheduler::jobThreadCallback(Scheduler * scheduler) {
	XOJ_CHECK_TYPE_OBJ(scheduler, Scheduler);

	while (scheduler->threadRunning) {
		// lock the whole scheduler
		g_mutex_lock(scheduler->schedulerMutex);

		g_mutex_lock(scheduler->blockRenderMutex);
		bool onlyNoneRenderJobs = false;
		glong diff = 1000;
		if (scheduler->blockRenderZoomTime) {
			GTimeVal time;
			g_get_current_time(&time);

			diff = g_time_val_diff(scheduler->blockRenderZoomTime, &time);
			if(diff <= 0) {
				g_free(scheduler->blockRenderZoomTime);
				scheduler->blockRenderZoomTime = NULL;
			} else {
				onlyNoneRenderJobs = true;
			}
		}
		g_mutex_unlock(scheduler->blockRenderMutex);

		g_mutex_lock(scheduler->jobQueueMutex);
		bool hasOnlyRenderJobs = false;
		Job * job = scheduler->getNextJobUnlocked(onlyNoneRenderJobs, &hasOnlyRenderJobs);
		if(job != NULL) {
			hasOnlyRenderJobs = false;
		}

		SDEBUG("get job: %ld\n", (long)job);

		if (!job) {
			// unlock the whole scheduler
			g_mutex_unlock(scheduler->schedulerMutex);

			if(hasOnlyRenderJobs) {
				if(scheduler->jobRenderThreadTimerId) {
					g_source_remove(scheduler->jobRenderThreadTimerId);
				}
				scheduler->jobRenderThreadTimerId = g_timeout_add(diff, (GSourceFunc)jobRenderThreadTimer, scheduler);
			}

			g_cond_wait(scheduler->jobQueueCond, scheduler->jobQueueMutex);
			g_mutex_unlock(scheduler->jobQueueMutex);

			continue;
		}

		SDEBUG("do job: %ld\n", (long)job);

		g_mutex_unlock(scheduler->jobQueueMutex);

		g_mutex_lock(scheduler->jobRunningMutex);

		job->execute();

		job->unref();
		g_mutex_unlock(scheduler->jobRunningMutex);

		// unlock the whole scheduler
		g_mutex_unlock(scheduler->schedulerMutex);

		SDEBUG("next\n", NULL);
	}

	SDEBUG("finished\n", NULL);

	return NULL;
}
