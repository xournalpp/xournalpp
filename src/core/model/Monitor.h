#include <cassert>
#include <mutex>
#include <optional>

template<class T>
class Monitor
{
public:
    template<typename ...Args>
    Monitor(Args&&... args);

    struct LockedMonitor
    {
        LockedMonitor(Monitor* monitor);
        LockedMonitor(Monitor* monitor, std::unique_lock<std::mutex> lock);
        T* operator->();
        void ReplaceModel(T model);
        private:
            Monitor* mon;
            std::unique_lock<std::mutex> lock;
    };

    LockedMonitor operator->();

    LockedMonitor lock();

    std::optional<LockedMonitor> try_lock();

    // TODO: remove after refactoring
    [[deprecated]] T& getUnsafeAccess();

private:
    T model;
    std::mutex mutex;
};
