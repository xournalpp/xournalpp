/*
 * Xournal++
 *
 * A scheduler for Xournal background tasks
 *
 * Some code from Evince project
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "gui/sidebar/previews/SidebarPreviewPage.h"
#include "gui/PageView.h"
#include "Scheduler.h"

#include <XournalType.h>

class XournalScheduler : public Scheduler
{
public:
	XournalScheduler();
	virtual ~XournalScheduler();
public:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSidebar(SidebarPreviewPage* preview);
	void removePage(PageView* view);
	void removeAllJobs();

	void addRepaintSidebar(SidebarPreviewPage* preview);
	void addRerenderPage(PageView* view);

	void finishTask();

private:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSource(void* source, JobType type, JobPriority priority);

	bool existsSource(void* source, JobType type, JobPriority priority);

private:
	XOJ_TYPE_ATTRIB;
};
