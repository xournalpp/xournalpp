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

#include "control/jobs/Scheduler.h"
#include "gui/sidebar/previews/page/SidebarPreviewPageEntry.h"
#include "gui/PageView.h"

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
	void removeSidebar(SidebarPreviewBaseEntry* preview);
	void removePage(XojPageView* view);

	/**
	 * Removes all PreviewJob%s / RenderJob%s scheduled to be run
	 */
	void removeAllJobs();

	void addRepaintSidebar(SidebarPreviewBaseEntry* preview);
	void addRerenderPage(XojPageView* view);

	/**
	 * Blocks until all currently running Job%s have been executed
	 */
	void finishTask();

private:
	/**
	 * Remove source, e.g. if a page is removed they don't need to repaint
	 */
	void removeSource(void* source, JobType type, JobPriority priority);

	bool existsSource(void* source, JobType type, JobPriority priority);

private:
	};
