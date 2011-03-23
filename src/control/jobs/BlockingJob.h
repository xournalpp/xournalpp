/*
 * Xournal++
 *
 * A job which is done in the GTK main thread, but the application is blocked
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __BLOCKINGJOB_H__
#define __BLOCKINGJOB_H__

#include "Job.h"
#include "../../util/XournalType.h"

#include <gtk/gtk.h>

class Control;

class BlockingJob: public Job {
public:
	BlockingJob(Control * control, const char * name);
	virtual ~BlockingJob();

public:
	void execute();

	virtual JobType getType();

protected:
	static bool finished(Control * control);

private:
	XOJ_TYPE_ATTRIB;

protected:
	Control * control;
};

#endif /* __BLOCKINGJOB_H__ */
