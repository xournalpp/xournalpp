/*
 * Xournal++
 *
 * A scheduler for Xournal background tasks
 *
 * Some code from Evince project
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */
// TODO: AA: type check

#ifndef __XOURNALSCHEDULER_H__
#define __XOURNALSCHEDULER_H__

#include "Scheduler.h"
#include "../../gui/sidebar/SidebarPreview.h"
#include "../../gui/PageView.h"


class XournalScheduler: public Scheduler {
public:
	XournalScheduler();
	virtual ~XournalScheduler();
public:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSidebar(SidebarPreview * preview);
	void removePage(PageView * view);


	void addRepaintSidebar(SidebarPreview * preview);
	void addRepaintPage(PageView * view);

	void finishTask();

private:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSource(void * source, JobType type, JobPriority priority);

	bool existsSource(void * source, JobType type, JobPriority priority);
};

#endif /* __XOURNALSCHEDULER_H__ */
