#include "Job.h"

#include <gtk/gtk.h>

Job::Job() { g_mutex_init(&this->refMutex); }

Job::~Job() = default;

void Job::unref() {
    g_mutex_lock(&this->refMutex);
    this->refCount--;

    if (this->refCount == 0) {
        g_mutex_unlock(&this->refMutex);
        delete this;
    } else {
        g_mutex_unlock(&this->refMutex);
    }
}

void Job::ref() {
    g_mutex_lock(&this->refMutex);
    this->refCount++;
    g_mutex_unlock(&this->refMutex);
}

void Job::deleteJob() {
    this->onDelete();

    if (this->afterRunId) {
        g_source_remove(this->afterRunId);
        this->unref();
    }
}

void Job::onDelete() {}

void Job::execute() { this->run(); }

auto Job::getSource() -> void* { return nullptr; }

auto Job::callAfterCallback(Job* job) -> bool {
    job->afterRun();

    job->afterRunId = 0;
    job->unref();
    return false;  // do not call again
}

void Job::callAfterRun() {
    if (this->afterRunId) {
        return;
    }

    this->ref();

    this->afterRunId = gdk_threads_add_idle(reinterpret_cast<GSourceFunc>(Job::callAfterCallback), this);
}

/**
 * After run will be called from UI Thread after the Job is finished
 *
 * All UI Stuff should happen here
 */
void Job::afterRun() {}
