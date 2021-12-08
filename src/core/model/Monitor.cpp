#include "Monitor.h"

/*
 * Credit to Mike Vine on StackOverflow.
 * https://stackoverflow.com/a/48408987
 */

template <class T>
template<typename ...Args>
Monitor<T>::Monitor(Args&&... args) : model(std::forward<Args>(args)...) {}


template <class T>
Monitor<T>::LockedMonitor::LockedMonitor(Monitor* monitor) : mon(monitor), lock(monitor->mutex) {}

template <class T>
T* Monitor<T>::LockedMonitor::operator->() { return &mon->model;}

template <class T>
void Monitor<T>::LockedMonitor::ReplaceModel(T model) {
    mon->model = model;
}


template <class T>
Monitor<T>::TryLockedMonitor::TryLockedMonitor(Monitor* monitor) {
    this->lock = std::unique_lock<std::mutex>(monitor->mutex, std::defer_lock);
    this->lockAcquired = lock.try_lock();
    if (this->lockAcquired) {
        this->mon = monitor;
    }
}

template <class T>
T* Monitor<T>::TryLockedMonitor::operator->() {
    assert(lockAcquired);
    return &mon->model;
};

template <class T>
void Monitor<T>::TryLockedMonitor::ReplaceModel(T model) {
    assert(lockAcquired);
    mon->model = model;
}


template <class T>
auto Monitor<T>::operator->() -> typename Monitor<T>::LockedMonitor { return LockedMonitor(this); }

template <class T>
auto Monitor<T>::lock() -> typename Monitor<T>::LockedMonitor { return LockedMonitor(this); }

template <class T>
auto Monitor<T>::tryLock() -> typename Monitor<T>::TryLockedMonitor { return TryLockedMonitor(this); }

template <class T>
T& Monitor<T>::getUnsafeAccess() { return model; }
