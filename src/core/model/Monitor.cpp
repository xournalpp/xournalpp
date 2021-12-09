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
Monitor<T>::LockedMonitor::LockedMonitor(Monitor* monitor, std::unique_lock<std::mutex> lock):
        mon(monitor), lock(std::move(lock)) {}

template <class T>
T* Monitor<T>::LockedMonitor::operator->() {
    return &mon->model;
}

template <class T>
void Monitor<T>::LockedMonitor::ReplaceModel(T model) {
    mon->model = model;
}


template <class T>
auto Monitor<T>::operator->() -> typename Monitor<T>::LockedMonitor { return LockedMonitor(this); }

template <class T>
auto Monitor<T>::lock() -> typename Monitor<T>::LockedMonitor { return LockedMonitor(this); }

template <class T>
auto Monitor<T>::try_lock() -> typename std::optional<Monitor<T>::LockedMonitor> {
    std::unique_lock<std::mutex> lock(this->mutex, std::defer_lock);
    if (lock.try_lock()) {
        return std::optional<Monitor<T>::LockedMonitor>{Monitor<T>::LockedMonitor(this, std::move(lock))};
    } else {
        return std::nullopt;
    }
}

// TODO: remove
template <class T>
T& Monitor<T>::getUnsafeAccess() { return model; }
