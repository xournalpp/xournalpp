/*
 * Xournal++
 *
 * Autosave job
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __AUTOSAVEJOB_H__
#define __AUTOSAVEJOB_H__

#include "Job.h"
#include <String.h>
#include <XournalType.h>

class Control;

class AutosaveJob: public Job
{
public:
	AutosaveJob(Control* control);

protected:
	virtual ~AutosaveJob();

public:
	virtual void run(bool noThreads);
	void afterRun();

	virtual JobType getType();

private:
	XOJ_TYPE_ATTRIB;


	Control* control;
	String error;
};

#endif /* __AUTOSAVEJOB_H__ */
