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

	this->thread = g_thread_create((GThreadFunc)jobThreadCallback, this, true, NULL);
}

Scheduler::~Scheduler() {
	XOJ_CHECK_TYPE(Scheduler);

	SDEBUG("Destroy scheduler\n", 0);

	stop();

	g_mutex_free(this->jobQueueMutex);
	g_mutex_free(this->jobRunningMutex);

	g_mutex_free(this->schedulerMutex);

	g_cond_free(this->jobQueueCond);

	Job * job = NULL;
	while(job = getNextJobUnlocked()) {
		job->unref();
	}

	XOJ_RELEASE_TYPE(Scheduler);
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

Job * Scheduler::getNextJobUnlocked() {
	XOJ_CHECK_TYPE(Scheduler);

	Job * job = NULL;

	for (int i = JOB_PRIORITY_URGENT; i < JOB_N_PRIORITIES; i++) {
		job = (Job *) g_queue_pop_head(this->jobQueue[i]);
		if (job) {
			break;
		}
	}

	return job;
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


gpointer Scheduler::jobThreadCallback(Scheduler * scheduler) {
	XOJ_CHECK_TYPE_OBJ(scheduler, Scheduler);

	while (scheduler->threadRunning) {
		// lock the whole scheduler
		g_mutex_lock(scheduler->schedulerMutex);

		g_mutex_lock(scheduler->jobQueueMutex);
		Job * job = scheduler->getNextJobUnlocked();
		SDEBUG("get job: %ld\n", (long)job);
		if (!job) {
			// unlock the whole scheduler
			g_mutex_unlock(scheduler->schedulerMutex);

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
