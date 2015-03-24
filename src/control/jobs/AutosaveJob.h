/*
 * Xournal++
 *
 * Autosave job
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __AUTOSAVEJOB_H__
#define __AUTOSAVEJOB_H__

#include "Job.h"
#include <StringUtils.h>
#include <XournalType.h>

class Control;

class AutosaveJob : public Job
{
public:
	AutosaveJob(Control* control);

protected:
	virtual ~AutosaveJob();

public:
	virtual void run();
	void afterRun();

	virtual JobType getType();

private:
	XOJ_TYPE_ATTRIB;


	Control* control;
	string error;
};

#endif /* __AUTOSAVEJOB_H__ */
