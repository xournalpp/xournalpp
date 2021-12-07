#include <cassert>

/*
 * Credit to Mike Vine on StackOverflow.
 * https://stackoverflow.com/a/48408987
 */
template<class T>
class Monitor
{
public:
    template<typename ...Args>
    Monitor(Args&&... args) : model(std::forward<Args>(args)...){}

    struct LockedMonitor
    {
        LockedMonitor(Monitor* monitor) : mon(monitor), lock(monitor->mutex) {}
        T* operator->() { return &mon->model;}
        private:
            Monitor* mon;
            std::unique_lock<std::mutex> lock;
    };

    struct TryLockedMonitor
    {
        TryLockedMonitor(Monitor* monitor) {
            this->lock = std::unique_lock<std::mutex>(monitor->mutex, std::defer_lock);
            this->lock_acquired = lock.try_lock();
            if (this->lock_acquired) {
                this->mon = monitor;
            }
        }
        T* operator->() {
            assert(lock_acquired);
            return &mon->model;
        };
        bool lock_acquired;
        private:
            Monitor* mon = NULL;
            std::unique_lock<std::mutex> lock;
    };

    LockedMonitor operator->() { return LockedMonitor(this); }

    LockedMonitor lock() { return LockedMonitor(this); }

    TryLockedMonitor try_lock() { return TryLockedMonitor(this); }

    T& get_unsafe_access() { return model; }

private:
    T model;
    std::mutex mutex;
};
