/*
 * Xournal++
 *
 * A job which is done in the GTK main thread, but the application is blocked
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Job.h"

#include <XournalType.h>
#include <StringUtils.h>

#include <gtk/gtk.h>

class Control;

class BlockingJob : public Job
{
public:
	BlockingJob(Control* control, string name);

protected:
	virtual ~BlockingJob();

public:
	void execute(bool noThreads);

	virtual JobType getType();

protected:
	static bool finished(Control* control);

private:
	XOJ_TYPE_ATTRIB;

protected:
	Control* control;
};
