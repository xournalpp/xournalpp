/*
 * Xournal++
 *
 * Basic memory RAII Guards for GLib objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>

#include <gtk/gtk.h>
namespace xoj::util {
inline namespace raii {

#define XOJ_GIO_GUARD_GENERATOR(class_name, type, deletion_func) \
    namespace detail {                                           \
    struct class_name##GuardDelete {                             \
        auto operator()(type* ptr) const -> void {               \
            if (*ptr) {                                          \
                deletion_func(*ptr);                             \
                *ptr = nullptr;                                  \
            }                                                    \
        }                                                        \
    };                                                           \
    } /* namespace detail */                                     \
    using class_name##Guard = std::unique_ptr<type, detail::class_name##GuardDelete>

#define XOJ_GIO_GUARD_GENERATOR_TYPE(type, deletion_func) XOJ_GIO_GUARD_GENERATOR(type, type, deletion_func)

XOJ_GIO_GUARD_GENERATOR(GError, GError*, g_error_free);
XOJ_GIO_GUARD_GENERATOR_TYPE(GStrv, g_strfreev);

}  // namespace raii
}  // namespace xoj::util