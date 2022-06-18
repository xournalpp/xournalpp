#include "Scheduler.h"

#include <cassert>    // for assert
#include <cinttypes>  // for PRId64, uint64_t

#include "control/jobs/Job.h"  // for Job, JOB_TYPE_RENDER

#include "config-debug.h"  // for DEBUG_SHEDULER

#ifdef DEBUG_SHEDULER
#define SDEBUG g_message
#else
#define SDEBUG(msg, ...)
#endif

Scheduler::Scheduler() {
    this->name = "Scheduler";

    // Queue
    this->jobQueue[JOB_PRIORITY_URGENT] = &this->queueUrgent;
    this->jobQueue[JOB_PRIORITY_HIGH] = &this->queueHigh;
    this->jobQueue[JOB_PRIORITY_LOW] = &this->queueLow;
    this->jobQueue[JOB_PRIORITY_NONE] = &this->queueNone;
}

Scheduler::~Scheduler() {
    SDEBUG("Destroy scheduler");

    if (this->jobRenderThreadTimerId) {
        g_source_remove(this->jobRenderThreadTimerId);
        this->jobRenderThreadTimerId = 0;
    }

    stop();

    Job* job = nullptr;
    while ((job = getNextJobUnlocked()) != nullptr) { job->unref(); }

    if (this->blockRenderZoomTime) {
        g_free(this->blockRenderZoomTime);
    }
}

void Scheduler::start() {
    SDEBUG("Starting scheduler");
    g_return_if_fail(this->thread == nullptr);

    this->thread = g_thread_new(name.c_str(), reinterpret_cast<GThreadFunc>(jobThreadCallback), this);
}

void Scheduler::stop() {
    SDEBUG("Stopping scheduler");

    if (!this->threadRunning) {
        return;
    }
    this->threadRunning = false;
    this->jobQueueCond.notify_all();

    if (this->thread) {
        g_thread_join(this->thread);
    }
}

void Scheduler::addJob(Job* job, JobPriority priority) {
    SDEBUG("Adding job...");

    {
        std::lock_guard lock{this->jobQueueMutex};

        job->ref();
        this->jobQueue[priority]->push_back(job);
    }

    SDEBUG("add job: %" PRId64 "; type: %" PRId64, (uint64_t)job, (uint64_t)job->getType());
    this->jobQueueCond.notify_all();
}

auto Scheduler::getNextJobUnlocked(bool onlyNotRender, bool* hasRenderJobs) -> Job* {
    Job* job = nullptr;

    for (int i = JOB_PRIORITY_URGENT; i < JOB_N_PRIORITIES; i++) {
        std::deque<Job*>& queue = *this->jobQueue[i];

        if (onlyNotRender) {
            for (auto it = queue.begin(); it != queue.end(); ++it) {
                job = *it;

                if (job->getType() != JOB_TYPE_RENDER) {
                    queue.erase(it);
                    return job;
                }

                if (hasRenderJobs != nullptr) {
                    *hasRenderJobs = true;
                }
            }
        } else if (!queue.empty()) {
            job = queue.front();
            queue.pop_front();
            assert(job != nullptr);

            return job;
        }
    }

    return nullptr;
}

/**
 * Locks the complete scheduler
 */
void Scheduler::lock() { this->schedulerMutex.lock(); }

/**
 * Unlocks the complete scheduler
 */
void Scheduler::unlock() { this->schedulerMutex.unlock(); }

#define ZOOM_WAIT_US_TIMEOUT 300000  // 0.3s

void Scheduler::blockRerenderZoom() {
    std::lock_guard lock{this->blockRenderMutex};

    if (this->blockRenderZoomTime == nullptr) {
        this->blockRenderZoomTime = g_new(GTimeVal, 1);
    }

    g_get_current_time(this->blockRenderZoomTime);
    g_time_val_add(this->blockRenderZoomTime, ZOOM_WAIT_US_TIMEOUT);
}

void Scheduler::unblockRerenderZoom() {
    {
        std::lock_guard lock{this->blockRenderMutex};

        g_free(this->blockRenderZoomTime);
        this->blockRenderZoomTime = nullptr;

        if (this->jobRenderThreadTimerId) {
            g_source_remove(this->jobRenderThreadTimerId);
            this->jobRenderThreadTimerId = 0;
        }
    }

    this->jobQueueCond.notify_all();
}

/**
 * g_time_val_diff:
 * @t1: time value t1
 * @t2: time value t2
 *
 * Calculates the time difference between t1 and t2 in milliseconds.
 * The result is positive if t1 is later than t2.
 *
 * Returns:
 * Time difference in microseconds
 */
auto g_time_val_diff(GTimeVal* t1, GTimeVal* t2) -> glong {
    g_assert(t1);
    g_assert(t2);
    return ((t1->tv_sec - t2->tv_sec) * G_USEC_PER_SEC + (t1->tv_usec - t2->tv_usec)) / 1000;
}

/**
 * If the Scheduler is blocking because we are zooming and there are only render jobs
 * we need to wakeup it later
 */
auto Scheduler::jobRenderThreadTimer(Scheduler* scheduler) -> bool {
    scheduler->jobRenderThreadTimerId = 0;

    {
        std::lock_guard lock{scheduler->blockRenderMutex};
        g_free(scheduler->blockRenderZoomTime);
        scheduler->blockRenderZoomTime = nullptr;
    }

    scheduler->jobQueueCond.notify_all();

    return false;
}

auto Scheduler::jobThreadCallback(Scheduler* scheduler) -> gpointer {
    while (scheduler->threadRunning) {
        // lock the whole scheduler
        std::unique_lock schedulerLock{scheduler->schedulerMutex};
        SDEBUG("Job Thread: Blocked scheduler.");

        bool onlyNonRenderJobs = false;
        glong diff = 1000;
        if (scheduler->blockRenderZoomTime) {
            std::lock_guard lock{scheduler->blockRenderMutex};
            SDEBUG("Zoom re-render blocking.");

            GTimeVal time;
            g_get_current_time(&time);

            diff = g_time_val_diff(scheduler->blockRenderZoomTime, &time);
            if (diff <= 0) {
                g_free(scheduler->blockRenderZoomTime);
                scheduler->blockRenderZoomTime = nullptr;
                SDEBUG("Ended zoom re-render blocking.");
            } else {
                onlyNonRenderJobs = true;
                SDEBUG("Rendering blocked: Only running non-rendering jobs.");
            }
        }

        Job* job;

        {
            std::unique_lock jobLock{scheduler->jobQueueMutex};
            SDEBUG("Job Thread: Locked job queue.");

            bool hasOnlyRenderJobs = false;
            job = scheduler->getNextJobUnlocked(onlyNonRenderJobs, &hasOnlyRenderJobs);
            if (job != nullptr) {
                hasOnlyRenderJobs = false;
            }

            SDEBUG("get job: %" PRId64, (uint64_t)job);

            if (job == nullptr) {
                // unlock the whole scheduler
                schedulerLock.unlock();

                if (hasOnlyRenderJobs) {
                    if (scheduler->jobRenderThreadTimerId) {
                        g_source_remove(scheduler->jobRenderThreadTimerId);
                    }
                    scheduler->jobRenderThreadTimerId = g_timeout_add(
                            static_cast<guint>(diff), reinterpret_cast<GSourceFunc>(jobRenderThreadTimer), scheduler);
                }

                scheduler->jobQueueCond.wait(jobLock);
                continue;
            }
        }

        // Run the job.
        {
            std::lock_guard lock{scheduler->jobRunningMutex};
            SDEBUG("do job: %" PRId64, (uint64_t)job);
            job->execute();
            job->unref();
        }

        SDEBUG("next");
    }

    SDEBUG("finished");

    return nullptr;
}
