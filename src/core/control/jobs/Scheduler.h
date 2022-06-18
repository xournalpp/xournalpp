/*
 * Xournal++
 *
 * A scheduler for background jobs
 *
 * Some code from Evince project
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <array>               // for array
#include <condition_variable>  // for condition_variable
#include <deque>               // for deque
#include <mutex>               // for mutex
#include <string>              // for string

#include <glib.h>  // for GThread, GTimeVal, gpointer

class Job;

/**
 * @file Scheduler.h
 * @brief A file containing the definition of the Scheduler
 */

/**
 * @enum JobPriority
 *
 * The priority of the job affects the order of execution:
 * Jobs with higher priority are processed before the
 * lower ones.
 */
enum JobPriority {
    /**
     * Urgent: used for rendering the current page
     */
    JOB_PRIORITY_URGENT,

    /**
     * High: used for rendering thumbnail ranges
     */
    JOB_PRIORITY_HIGH,

    /**
     * Low: used for rendering of pages not in the current range
     */
    JOB_PRIORITY_LOW,

    /**
     * None: used for any other job (loading / saving / printing...)
     */
    JOB_PRIORITY_NONE,

    /**
     * The number of priorities
     */
    JOB_N_PRIORITIES
};


class Scheduler {
public:
    Scheduler();
    virtual ~Scheduler();

public:
    /**
     * Adds a Job to the Scheduler
     *
     * @param Job      the job
     * @param priority the desired priority
     *
     * The Job is now owned by the scheduler, and automatically freed if it is done
     */
    void addJob(Job* job, JobPriority priority);

    void start();
    void stop();

    /**
     * Locks the complete scheduler
     */
    void lock();

    /**
     * Unlocks the complete scheduler
     */
    void unlock();

    /**
     * Don't render the next X ms so the scrolling performance is better
     */
    void blockRerenderZoom();

    /**
     * Remove the blocked rendering manually
     */
    void unblockRerenderZoom();

private:
    static gpointer jobThreadCallback(Scheduler* scheduler);
    Job* getNextJobUnlocked(bool onlyNotRender = false, bool* hasRenderJobs = nullptr);

    static bool jobRenderThreadTimer(Scheduler* scheduler);

protected:
    bool threadRunning = true;

    int jobRenderThreadTimerId = 0;

    GThread* thread = nullptr;

    std::condition_variable jobQueueCond{};
    std::mutex jobQueueMutex{};
    std::mutex schedulerMutex{};

    /**
     * This is need to be sure there is no job running if we delete a page.
     * If a job is, we may access deleted memory.
     */
    std::mutex jobRunningMutex{};

    /**
     * Jobs of each priority. New jobs
     * are added to the back of each queue.
     */
    std::deque<Job*> queueUrgent{};
    std::deque<Job*> queueHigh{};
    std::deque<Job*> queueLow{};
    std::deque<Job*> queueNone{};

    /**
     * Map (JobPriority) -> queue of that priority.
     * For example,
     *   jobQueue[JOB_PRIORITY_URGENT] is &queueUrgent.
     */
    std::array<std::deque<Job*>*, JOB_N_PRIORITIES> jobQueue{};

    GTimeVal* blockRenderZoomTime = nullptr;
    std::mutex blockRenderMutex{};

    std::string name;
};
