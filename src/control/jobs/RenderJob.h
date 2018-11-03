/*
 * Xournal++
 *
 * A job which redraws a page or a page region
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Job.h"

#include <XournalType.h>

class Rectangle;
class PageView;

class RenderJob : public Job
{
public:
	RenderJob(PageView* view);

protected:
	virtual ~RenderJob();

public:
	virtual JobType getType();

	void* getSource();

	void run(bool noThreads);

public:
	static void rerenderRectangle(RenderJob* renderJob, Rectangle* rect, bool noThreads);

private:
	void rerenderRectangle(Rectangle* rect, bool noThreads);

private:
	XOJ_TYPE_ATTRIB;

	PageView* view;
};
