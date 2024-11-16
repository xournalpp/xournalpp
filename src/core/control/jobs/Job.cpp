#include "Job.h"

#include <gdk/gdk.h>  // for gdk_threads_add_idle
#include <glib.h>     // for g_source_remove

#include "util/glib_casts.h"  // for wrap_for_once_v


Job::Job(): refCount(1) {}

Job::~Job() = default;

void Job::unref() {
    if (refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete this;
    }
}

void Job::ref() {
    refCount.fetch_add(1, std::memory_order_relaxed);
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
    this->afterRunId = g_idle_add(xoj::util::wrap_for_once_v<Job::callAfterCallback>, this);
}

void Job::afterRun() {}
