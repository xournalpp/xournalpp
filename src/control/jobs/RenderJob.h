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
#include "../../gui/PageView.h"

class RenderJob: public Job {
public:
	RenderJob(PageView * view);
	virtual ~RenderJob();

public:
	virtual JobType getType();

	void * getSource();

	void run();

private:
	void repaintRectangle(Rectangle * rect, double zoom);

private:
	PageView * view;
};

#endif /* __RENDERJOB_H__ */
