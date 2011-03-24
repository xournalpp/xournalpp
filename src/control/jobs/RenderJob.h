/*
 * Xournal++
 *
 * A job which redraws a page or a page region
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RENDERJOB_H__
#define __RENDERJOB_H__

#include "Job.h"
#include <gtk/gtk.h>

#include "../../util/XournalType.h"

class Rectangle;
class PageView;

class RenderJob: public Job {
public:
	RenderJob(PageView * view);
	virtual ~RenderJob();

public:
	virtual JobType getType();

	void * getSource();

	void run();
	virtual void afterRun();

public:
	static void repaintRectangle(RenderJob * renderJob, Rectangle * rect);

private:
	void rerenderRectangle(Rectangle * rect);

private:
	XOJ_TYPE_ATTRIB;

	PageView * view;
	bool repaintComplete;
	GList * repaintRect;
};

#endif /* __RENDERJOB_H__ */
