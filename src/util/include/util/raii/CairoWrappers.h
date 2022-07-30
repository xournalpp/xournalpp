/*
 * Xournal++
 *
 * RAII wrappers for C library classes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <utility>

#include <cairo.h>

#include "CLibrariesSPtr.h"
#include "IdentityFunction.h"

namespace xoj::util {

inline namespace raii {
namespace specialization {
class CairoHandler {
public:
    constexpr static auto ref = cairo_reference;
    constexpr static auto unref = cairo_destroy;
    constexpr static auto adopt = identity<cairo_t>;
};

class CairoSurfaceHandler {
public:
    constexpr static auto ref = cairo_surface_reference;
    constexpr static auto unref = cairo_surface_destroy;
    constexpr static auto adopt = identity<cairo_surface_t>;
};
};  // namespace specialization

using CairoSPtr = CLibrariesSPtr<cairo_t, raii::specialization::CairoHandler>;
using CairoSurfaceSPtr = CLibrariesSPtr<cairo_surface_t, raii::specialization::CairoSurfaceHandler>;

/**
 * @brief cairo_save(cr)/cairo_restore(cr) RAII implementation
 */
class CairoSaveGuard {
public:
    CairoSaveGuard() = delete;
    CairoSaveGuard(cairo_t* cr): cr(cr) { cairo_save(cr); }
    ~CairoSaveGuard() { cairo_restore(cr); }

    CairoSaveGuard(const CairoSaveGuard&) = delete;
    CairoSaveGuard(CairoSaveGuard&&) = delete;
    CairoSaveGuard& operator=(const CairoSaveGuard&) = delete;
    CairoSaveGuard& operator=(CairoSaveGuard&&) = delete;

private:
    cairo_t* cr;
};

};  // namespace raii
};  // namespace xoj::util
