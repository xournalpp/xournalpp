#include "Monitor.h"

/*
 * Credit to Mike Vine on StackOverflow.
 * https://stackoverflow.com/a/48408987
 */

Monitor::Monitor(Args&&... args) : model(std::forward<Args>(args)...) {}

struct Monitor::LockedMonitor
{
    LockedMonitor(Monitor* monitor) : mon(monitor), lock(monitor->mutex) {}
    T* operator->() { return &mon->model;}
    void ReplaceModel(T model) {
        this->model = model;
    }
};

struct Monitor::TryLockedMonitor
    {
        TryLockedMonitor(Monitor* monitor) {
            this->lock = std::unique_lock<std::mutex>(monitor->mutex, std::defer_lock);
            this->lockAcquired = lock.try_lock();
            if (this->lockAcquired) {
                this->mon = monitor;
            }
        }
        T* operator->() {
            assert(lockAcquired);
            return &mon->model;
        };
        void ReplaceModel(T model) {
            assert(lockAcquired);
            this->model = model;
        }
    };

Monitor::LockedMonitor operator->() { return LockedMonitor(this); }

Monitor::LockedMonitor lock() { return LockedMonitor(this); }

Monitor::TryLockedMonitor tryLock() { return TryLockedMonitor(this); }

T& Monitor::getUnsafeAccess() { return model; }
