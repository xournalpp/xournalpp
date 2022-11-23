/*
 * Xournal++
 *
 * Template class for a dispatch pool and listeners
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

namespace xoj::util {

/**
 * @brief Utility class for dispatching a method call to multiple "listeners".
 *
 * The listeners must implement functions
 *      void on(Arg&&...);
 * that the dispatcher will call through dispatch() and/or functions
 *      void deleteOn(Arg&&...);
 * that the dispatcher will call through dispatchAndClear();
 *
 * The `on()`s must not delete the listener, or even remove it from the pool.
 * The `deleteOn()`s must make sure the listener no longer references the pool (for instance by deleting the listener)
 */
template <class ListenerT>
class DispatchPool final {
public:
    using listener_type = ListenerT;

    /**
     * @brief Invokes the `on()` method of all registered `ListenerT`s.
     */
    template <typename... Args>
    void dispatch(Args&&... args) const {
        for (auto* v: this->pool) {
            v->on(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Invokes the `deleteOn()` method of all registered `ListenerT`s and clears the pool.
     * A call to v->deleteOn() is supposed to delete the listener v
     * (or at the very least remove any reference to the dispatcher in the listener v)
     */
    template <typename... Args>
    void dispatchAndClear(Args&&... args) const {
        // We cannot iterate on the this->pool: the listener's deletion would trigger calls to DispatchPool::remove()
        // leading to possible segfaults.
        auto p = std::move(this->pool);
        for (auto* v: p) {
            v->deleteOn(std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Adds a new listener to the pool.
     * The listener must not already be registered.
     */
    void add(listener_type* v) {
        assert(v != nullptr && "Adding nullptr listener");
        assert(std::find(this->pool.begin(), this->pool.end(), v) == this->pool.end() && "Listener is already listed");
        this->pool.emplace_back(v);
    }

    void remove(listener_type* v) {
        assert(v != nullptr && "Removing nullptr listener");
        auto it = std::find(this->pool.begin(), this->pool.end(), v);
        if (it != this->pool.end()) {
            this->pool.erase(it);
        }
    }

    [[nodiscard]] bool empty() const { return pool.empty(); }
    [[nodiscard]] const listener_type& front() const { return *pool.front(); }

private:
    std::vector<listener_type*> pool;
};

/**
 * @brief CRTP-style class for listener
 * Usage:
 *  class A: Listener<A> {
 *      (virtual) ~A() { unregisterFromPool(); }
 *      void on(...);  // Signal receiver
 *  };
 *
 * WARNING: Always unregisterFromPool() in derived class destructor.
 *
 * For listening to several type of dispatchers do:
 *     class A : public Listener<A> {...};  // virtual void on(...); in here
 *     class B : public Listener<B> {...};  // virtual void on(...); in here - types must differ from the ones in A
 *     class C : public A, public B {...};  // overload everything here
 */
template <class T>
class Listener {
private:
    // Keep the constructor private: only T can inherit Listener<T> (for the static_cast<T*>(this) below).
    Listener() = default;
    friend T;

    using pool_type = xoj::util::DispatchPool<T>;

public:
    /**
     * @brief Register to a new dispatch pool. Unregisters from any pool the listener was previously registered to.
     */
    void registerToPool(const std::shared_ptr<pool_type>& newpool) {
        if (auto p = this->pool.lock()) {
            p->remove(static_cast<T*>(this));
        }
        newpool->add(static_cast<T*>(this));
        this->pool = newpool;
    }

    /**
     * @brief Unregisters from the current pool (if any).
     */
    void unregisterFromPool() {
        if (auto p = this->pool.lock()) {
            p->remove(static_cast<T*>(this));
        }
        this->pool.reset();
    }

    [[nodiscard]] auto getPool() const -> std::shared_ptr<pool_type> { return this->pool.lock(); }

private:
    std::weak_ptr<pool_type> pool;
};
};  // namespace xoj::util
