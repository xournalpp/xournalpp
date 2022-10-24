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

#include <glib-object.h>
#include <gtk/gtk.h>

#include "CLibrariesSPtr.h"

namespace xoj::util {

inline namespace raii {
namespace specialization {

template <class object_type>
class GObjectHandler {
public:
    static object_type* ref(object_type* p) { return static_cast<object_type*>(g_object_ref(p)); }
    constexpr static auto unref = g_object_unref;
    static object_type* ref_sink(object_type* p) { return static_cast<object_type*>(g_object_ref_sink(p)); }
    static object_type* adopt(object_type* p) {
#if (GLIB_MAJOR_VERSION > 2 || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION >= 70))
        return static_cast<object_type*>(g_object_take_ref(p));
#else
        return g_object_is_floating(p) ? static_cast<object_type*>(g_object_ref_sink(p)) : p;
#endif
    }
};
};  // namespace specialization

template <typename GlibClass>
using GObjectSPtr = CLibrariesSPtr<GlibClass, raii::specialization::GObjectHandler<GlibClass>>;

using WidgetSPtr = GObjectSPtr<GtkWidget>;

};  // namespace raii
};  // namespace xoj::util
