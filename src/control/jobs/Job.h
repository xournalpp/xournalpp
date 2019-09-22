/*
 * Xournal++
 *
 * A job which is done
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

enum JobType
{
	JOB_TYPE_BLOCKING, JOB_TYPE_PREVIEW, JOB_TYPE_RENDER, JOB_TYPE_AUTOSAVE
};

class Job
{
public:
	Job();

protected:
	virtual ~Job();

public:

	/**
	 * Unref the Job, the initial refcount is set to 1 on creation
	 */
	void unref();

	/**
	 * Increase the refcount
	 */
	void ref();

	/**
	 * Delete Job because e.g. the source was removed
	 */
	void deleteJob();

public:
	virtual JobType getType() = 0;

public:
	/**
	 * this method is called
	 */
	virtual void execute();

	virtual void* getSource();

protected:
	/**
	 * override this method
	 */
	virtual void run() = 0;

	/**
	 * This method should be called as _last_ operation in run
	 *
	 * If you call it in another position the object will be deleted before run is finished!
	 */
	void callAfterRun();

	/**
	 * After run will be called from UI Thread after the Job is finished
	 *
	 * All UI Stuff should happen here
	 */
	virtual void afterRun();

private:
	static bool callAfterCallback(Job* job);

private:
	int afterRunId = 0;

	int refCount = 1;
	GMutex refMutex;
};
