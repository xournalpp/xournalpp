/*
 * Xournal++
 *
 * A job which is done
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __JOB_H__
#define __JOB_H__

#include "../../util/MemoryCheck.h"

enum JobType {
	JOB_TYPE_BLOCKING, JOB_TYPE_PREVIEW, JOB_TYPE_RENDER
};

class Job: public MemoryCheckObject {
public:
	Job();

protected:
	virtual ~Job();

public:
	void free();

public:
	virtual JobType getType() = 0;

public:
	/**
	 * this method is called
	 */
	virtual void execute();

	virtual void * getSource();

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
	 * override this method
	 */
	virtual void afterRun();

private:
	static bool callAfterCallback(Job * job);

private:
	int afterRunId;
};

#endif /* __JOB_H__ */
