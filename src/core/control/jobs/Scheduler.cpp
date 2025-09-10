#include "Scheduler.h"

#include <cinttypes>  // for PRId64
#include <cstdint>    // for uint64_t

#include "control/jobs/Job.h"  // for Job, JOB_TYPE_RENDER
#include "util/Assert.h"       // for xoj_assert
#include "util/glib_casts.h"   // for wrap_for_once_v

#include "config-debug.h"  // for DEBUG_SCHEDULER

#ifdef DEBUG_SCHEDULER
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

    if (auto id = this->jobRenderThreadTimerId.exchange(0); id != 0) {
        g_source_remove(id);
    }

    stop();

    Job* job = nullptr;
    while ((job = getNextJobUnlocked()) != nullptr) {
        job->unref();
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

    for (size_t i = JOB_PRIORITY_URGENT; i < JOB_N_PRIORITIES; i++) {
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
            xoj_assert(job != nullptr);

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

void Scheduler::blockRerenderZoom() { this->blockRenderZoomTime = g_get_monotonic_time() + ZOOM_WAIT_US_TIMEOUT; }

void Scheduler::unblockRerenderZoom() {
    this->blockRenderZoomTime = 0;

    if (auto id = this->jobRenderThreadTimerId.exchange(0); id != 0) {
        g_source_remove(id);
    }

    this->jobQueueCond.notify_all();
}

/**
 * If the Scheduler is blocking because we are zooming and there are only render jobs
 * we need to wakeup it later
 */
auto Scheduler::jobRenderThreadTimer(Scheduler* scheduler) -> bool {
    scheduler->jobRenderThreadTimerId = 0;
    scheduler->blockRenderZoomTime = 0;

    scheduler->jobQueueCond.notify_all();

    return false;
}

auto Scheduler::jobThreadCallback(Scheduler* scheduler) -> gpointer {
    while (scheduler->threadRunning) {
        // lock the whole scheduler
        std::unique_lock schedulerLock{scheduler->schedulerMutex};
        SDEBUG("Job Thread: Blocked scheduler.");

        bool onlyNonRenderJobs = false;
        gint64 diff = 1000;
        if (scheduler->blockRenderZoomTime) {
            SDEBUG("Zoom re-render blocking.");

            diff = (scheduler->blockRenderZoomTime - g_get_monotonic_time()) / 1000;
            if (diff <= 0) {
                scheduler->blockRenderZoomTime = 0;
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
                    if (auto id = scheduler->jobRenderThreadTimerId.exchange(g_timeout_add(
                                static_cast<guint>(diff), xoj::util::wrap_for_once_v<jobRenderThreadTimer>, scheduler));
                        id != 0) {
                        g_source_remove(id);
                    }
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
