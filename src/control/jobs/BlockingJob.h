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
#include "ProgressListener.h"

#include <gtk/gtk.h>

class Control;

class BlockingJob: public Job, public ProgressListener {
public:
	BlockingJob(Control * control, const char * name);
	virtual ~BlockingJob();

public:
	virtual void execute();
	virtual void finished();

	virtual JobType getType();

public:
	void setMaximumState(int max);
	void setCurrentState(int state);

protected:
	Control * control;

	GtkWidget * statusbar;
	GtkLabel * lbState;
	GtkProgressBar * pgState;

private:
	int maxState;
};

#endif /* __BLOCKINGJOB_H__ */
