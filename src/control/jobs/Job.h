/*
 * Xournal++
 *
 * A job wich is done
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
	JOB_TYPE_BLOCKING,
	JOB_TYPE_PREVIEW,
	JOB_TYPE_RENDER
};

class Job : public MemoryCheckObject {
public:
	Job();
	virtual ~Job();

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
};

#endif /* __JOB_H__ */
