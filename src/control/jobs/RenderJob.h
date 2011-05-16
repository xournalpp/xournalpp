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

#include <XournalType.h>

class Rectangle;
class PageView;

class RenderJob: public Job {
public:
	RenderJob(PageView * view);

protected:
	virtual ~RenderJob();

public:
	virtual JobType getType();

	void * getSource();

	void run();

public:
	static void rerenderRectangle(RenderJob * renderJob, Rectangle * rect);

private:
	void rerenderRectangle(Rectangle * rect);

private:
	XOJ_TYPE_ATTRIB;

	PageView * view;
};

#endif /* __RENDERJOB_H__ */
