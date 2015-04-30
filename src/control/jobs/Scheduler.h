/*
 * Xournal++
 *
 * A scheduler for background jobs
 *
 * Some code from Evince project
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Job.h"

#include <XournalType.h>

#include <glib.h>

typedef enum
{
	/**
	 * Rendering current page range
	 */
	JOB_PRIORITY_URGENT,

	/**
	 * Rendering current thumbnail range
	 */
	JOB_PRIORITY_HIGH,

	/**
	 * Rendering pages not in current range
	 */
	JOB_PRIORITY_LOW,

	/**
	 * Any other job: load, save, print, ...
	 */
	JOB_PRIORITY_NONE,

	JOB_N_PRIORITIES
} JobPriority;

class Scheduler
{
public:
	Scheduler();
	virtual ~Scheduler();

public:
	/**
	 * Adds a job
	 *
	 * The Job is now owned by the scheduler, and automatically freed if it is done
	 */
	void addJob(Job* job, JobPriority priority);

	void start();
	void stop();

	/**
	 * Locks the complete scheduler
	 */
	void lock();

	/**
	 * Unlocks the complete scheduler
	 */
	void unlock();

	/**
	 * Don't render the next X ms so the scrolling performance is better
	 */
	void blockRerenderZoom();

	/**
	 * Remove the blocked rendering manually
	 */
	void unblockRerenderZoom();

private:
	static gpointer jobThreadCallback(Scheduler* scheduler);
	Job* getNextJobUnlocked(bool onlyNotRender = false, bool* hasRenderJobs = NULL);

	static bool jobRenderThreadTimer(Scheduler* scheduler);

protected:
	XOJ_TYPE_ATTRIB;

	bool threadRunning;

	int jobRenderThreadTimerId;

	GThread* thread;

	GCond jobQueueCond;
	GMutex jobQueueMutex;

	GMutex schedulerMutex;

	/**
	 * This is need to be sure there is no job running if we delete a page, else we may access delete memory...
	 */
	GMutex jobRunningMutex;

	GQueue queueUrgent;
	GQueue queueHigh;
	GQueue queueLow;
	GQueue queueNone;

	GQueue* jobQueue[JOB_N_PRIORITIES];

	GTimeVal * blockRenderZoomTime;
	GMutex blockRenderMutex;

	const char *name;
};
