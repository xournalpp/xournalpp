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

#ifndef __XOURNALSCHEDULER_H__
#define __XOURNALSCHEDULER_H__

#include "Scheduler.h"
#include "../../gui/sidebar/previews/SidebarPreviewPage.h"
#include "../../gui/PageView.h"
#include <XournalType.h>

class XournalScheduler: public Scheduler {
public:
	XournalScheduler();
	virtual ~XournalScheduler();
public:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSidebar(SidebarPreviewPage * preview);
	void removePage(PageView * view);
	void removeAllJobs();

	void addRepaintSidebar(SidebarPreviewPage * preview);
	void addRerenderPage(PageView * view);

	void finishTask();

private:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSource(void * source, JobType type, JobPriority priority);

	bool existsSource(void * source, JobType type, JobPriority priority);

private:
	XOJ_TYPE_ATTRIB;
};

#endif /* __XOURNALSCHEDULER_H__ */
