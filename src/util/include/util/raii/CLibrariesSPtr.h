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

    using handler_type = H;

private:
    static auto safeRef(T* ptr) -> T* { return ptr ? H::ref(ptr) : ptr; }
    static auto safeUnref(T* ptr) -> void {
        if (ptr) {
            H::unref(ptr);
        }
    }
    static auto safeSinkRef(T* ptr) -> T* { return ptr ? H::sink_ref(ptr) : ptr; }
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

    constexpr static struct Adopt {
    } adopt = Adopt();
    constexpr static struct Ref {
    } ref = Ref();
    constexpr static struct RefSink {
    } refsink = RefSink();

    CLibrariesSPtr(T* p, Adopt = adopt): p(H::adopt(p)) {}
    CLibrariesSPtr(T* p, Ref): p(safeRef(p)) {}
    CLibrariesSPtr(T* p, RefSink): p(safeSinkRef(p)) {}

    void reset(T* other = nullptr, Adopt = adopt) { safeReset(p, other); }
    void reset(T* other, Ref) { safeReset(p, safeRef(other)); }
    void reset(T* other, RefSink) { safeReset(p, safeRefSink(other)); }

    operator bool() const { return p != nullptr; }

    T* get() { return p; }
    const T* get() const { return p; }

    T* release() { return std::exchange(p, nullptr); }

    T* operator->() { return p; }
    const T* operator->() const { return p; }

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
