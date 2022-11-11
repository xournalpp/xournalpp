/*
 * Xournal++
 *
 * RAII smart pointers for C library classes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <utility>

namespace xoj::util {
/**
 * @brief Placeholder type for C-lib smart pointer wrappers' contructors and reset methods: adopt the element
 */
constexpr static struct Adopt {
} adopt = Adopt();

/**
 * @brief Placeholder type for C-lib smart pointer wrappers' contructors and reset methods: add a ref to the element
 */
constexpr static struct Ref {
} ref = Ref();

/**
 * @brief Placeholder type for C-lib smart pointer wrappers' contructors and reset methods: ref_sink the element
 */
constexpr static struct RefSink {
} refsink = RefSink();

/**
 * @brief Simple template class for RAII smart pointer (aimed at Cairo/GTK/Poppler ref-counting classes)
 * @param T The wrapper will store a pointer of type T
 * @param H Handler class containing at least
 *              static T* ref(T*):
 *              static void unref(T*);
 *              static T* adopt(T*);  // What to do to a pointer when we adopt an instance (typically the identity)
 *          and optionally (for floating refs)
 *              static T* ref_sink(T*);
 */
template <typename T, class H>
class CLibrariesSPtr {
public:
    CLibrariesSPtr() = default;
    CLibrariesSPtr(std::nullptr_t) {}

    using handler_type = H;

private:
    static auto safeAdopt(T* ptr) -> T* { return ptr ? H::adopt(ptr) : nullptr; }
    static auto safeRef(T* ptr) -> T* { return ptr ? H::ref(ptr) : nullptr; }
    static auto safeUnref(T* ptr) -> void {
        if (ptr) {
            H::unref(ptr);
        }
    }
    static auto safeRefSink(T* ptr) -> T* { return ptr ? H::ref_sink(ptr) : nullptr; }
    static auto safeReset(T*& ptr, T* val) -> void { safeUnref(std::exchange(ptr, val)); }

public:
    ~CLibrariesSPtr() { safeUnref(p); }

    CLibrariesSPtr(const CLibrariesSPtr& other) { p = safeRef(other.p); }
    CLibrariesSPtr(CLibrariesSPtr&& other) { p = std::exchange(other.p, nullptr); }

    CLibrariesSPtr& operator=(const CLibrariesSPtr& other) {
        if (this != &other) {
            safeReset(p, safeRef(other.p));
        }
        return *this;
    }
    CLibrariesSPtr& operator=(CLibrariesSPtr&& other) {
        if (this != &other) {
            safeReset(p, std::exchange(other.p, nullptr));
        }
        return *this;
    }

    explicit CLibrariesSPtr(T* p, Adopt): p(safeAdopt(p)) {}
    explicit CLibrariesSPtr(T* p, Ref): p(safeRef(p)) {}
    explicit CLibrariesSPtr(T* p, RefSink): p(safeRefSink(p)) {}

    void reset() { safeReset(p, nullptr); }
    void reset(T* other, Adopt) { safeReset(p, safeAdopt(other)); }
    void reset(T* other, Ref) { safeReset(p, safeRef(other)); }
    void reset(T* other, RefSink) { safeReset(p, safeRefSink(other)); }

    operator bool() const { return p != nullptr; }

    T* get() const { return p; }

    T* release() { return std::exchange(p, nullptr); }

    T* operator->() const { return p; }

    void swap(CLibrariesSPtr& other) { std::swap(p, other.p); }

private:
    T* p = nullptr;
};
};  // namespace xoj::util

namespace std {
template <typename T, class H>
void swap(xoj::util::CLibrariesSPtr<T, H>& first, xoj::util::CLibrariesSPtr<T, H>& second) {
    first.swap(second);
}
};  // namespace std
