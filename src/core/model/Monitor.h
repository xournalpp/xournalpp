#include <cassert>
#include <mutex>

template<class T>
class Monitor
{
public:
    template<typename ...Args>
    Monitor(Args&&... args);

    struct LockedMonitor
    {
        LockedMonitor(Monitor* monitor);
        T* operator->();
        void ReplaceModel(T model);
        private:
            Monitor* mon;
            std::unique_lock<std::mutex> lock;
    };

    struct TryLockedMonitor
    {
        TryLockedMonitor(Monitor* monitor);
        T* operator->();
        void ReplaceModel(T model);
        bool lockAcquired;
        private:
            Monitor* mon = nullptr;
            std::unique_lock<std::mutex> lock;
    };


    LockedMonitor operator->();

    LockedMonitor lock();

    TryLockedMonitor tryLock();

    T& getUnsafeAccess();

private:
    T model;
    std::mutex mutex;
};
