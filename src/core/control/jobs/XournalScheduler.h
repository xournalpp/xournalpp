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

#include <optional>

#include "control/jobs/Job.h"  // for JobType

#include "Scheduler.h"  // for JobPriority, Scheduler

class SidebarPreviewBaseEntry;
class XojPageView;
class Recolor;

class XournalScheduler: public Scheduler {
public:
    XournalScheduler();
    ~XournalScheduler() override;

public:
    /**
     * Remove source, e.g. if a page is removed they don't need to repaint.
     * Blocks until all jobs that could be using the source have finished.
     */
    void removeSidebar(SidebarPreviewBaseEntry* preview);
    void removePage(XojPageView* view);

    /**
     * Removes all PreviewJob%s / RenderJob%s scheduled to be run
     */
    void removeAllJobs();

    void addRepaintSidebar(SidebarPreviewBaseEntry* preview, std::optional<Recolor> recolor);
    void addRerenderPage(XojPageView* view);

    /**
     * Blocks until all currently running Job%s have been executed
     */
    void finishTask();

private:
    /**
     * Remove source, e.g. if a page is removed they don't need to repaint
     */
    void removeSource(void* source, JobType type, JobPriority priority, bool awaitFinishTask = true);

    bool existsSource(void* source, JobType type, JobPriority priority);

private:
};
