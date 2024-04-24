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

#include <memory>

#include <gtk/gtk.h>


namespace xoj::util {

inline namespace raii {
namespace specialization {
struct GtkWindowDeleter {
    void operator()(GtkWindow* w) {
        if (w) {
            gtk_window_destroy(w);
        }
    }
};
};  // namespace specialization
using GtkWindowUPtr = std::unique_ptr<GtkWindow, specialization::GtkWindowDeleter>;


};  // namespace raii
};  // namespace xoj::util
