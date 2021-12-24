#include "XournalScheduler.h"

#include "PreviewJob.h"
#include "RenderJob.h"

XournalScheduler::XournalScheduler() { this->name = "XournalScheduler"; }

XournalScheduler::~XournalScheduler() = default;

void XournalScheduler::removeSidebar(SidebarPreviewBaseEntry* preview) {
    // Wait for running jobs to finish: Currently running jobs may still be
    //  using `preview`, and, as such, it is not completely removed.
    bool waitForTaskCompletion = true;
    removeSource(preview, JOB_TYPE_PREVIEW, JOB_PRIORITY_HIGH, waitForTaskCompletion);
}

void XournalScheduler::removePage(XojPageView* view) { removeSource(view, JOB_TYPE_RENDER, JOB_PRIORITY_URGENT); }

void XournalScheduler::removeAllJobs() {
    std::lock_guard lock{this->jobQueueMutex};

    for (int priority = JOB_PRIORITY_URGENT; priority < JOB_N_PRIORITIES; priority++) {
        std::deque<Job*>& queue = *this->jobQueue[priority];
        auto it = queue.begin();

        while (it != queue.end()) {
            Job* job = *it;

            // Only remove PREVIEW and RENDER jobs; we aren't
            // responsible for other types of jobs.
            JobType type = job->getType();
            if (type == JOB_TYPE_PREVIEW || type == JOB_TYPE_RENDER) {
                job->deleteJob();

                it = queue.erase(it);

                job->unref();
                job = nullptr;
            } else {
                ++it;
            }
        }
    }
}

void XournalScheduler::finishTask() { std::lock_guard lock{this->jobRunningMutex}; }

void XournalScheduler::removeSource(void* source, JobType type, JobPriority priority, bool awaitFinishTask) {
    {
        std::lock_guard lock{this->jobQueueMutex};
        std::deque<Job*>& queue = *this->jobQueue[priority];

        auto it = queue.begin();

        while (it != queue.end()) {
            Job* job = *it;

            if (job->getType() == type && job->getSource() == source) {
                it = queue.erase(it);

                job->deleteJob();
                job->unref();
                job = nullptr;
            } else {
                ++it;
            }
        }
    }

    // wait until the last job is done
    // we can be sure we don't access "source"
    if (awaitFinishTask) {
        finishTask();
    }
}

auto XournalScheduler::existsSource(void* source, JobType type, JobPriority priority) -> bool {
    bool exists = false;
    std::lock_guard lock{this->jobQueueMutex};

    for (Job* job: *this->jobQueue[priority]) {
        if (job->getType() == type && job->getSource() == source) {
            exists = true;
            break;
        }
    }

    return exists;
}

void XournalScheduler::addRepaintSidebar(SidebarPreviewBaseEntry* preview) {
    if (existsSource(preview, JOB_TYPE_PREVIEW, JOB_PRIORITY_HIGH)) {
        return;
    }

    auto* job = new PreviewJob(preview);
    addJob(job, JOB_PRIORITY_HIGH);
    job->unref();
}

void XournalScheduler::addRerenderPage(XojPageView* view) {
    if (existsSource(view, JOB_TYPE_RENDER, JOB_PRIORITY_URGENT)) {
        return;
    }

    auto* job = new RenderJob(view);
    addJob(job, JOB_PRIORITY_URGENT);
    job->unref();
}
