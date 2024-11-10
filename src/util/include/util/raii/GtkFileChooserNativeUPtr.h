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

#include "util/gtk4_helper.h"

namespace xoj::util {

inline namespace raii {
namespace specialization {
struct GtkFileChooserNativeDeleter {
    void operator()(GtkFileChooserNative* w) {
        if (w) {
            g_object_unref(w);
        }
    }
};
};  // namespace specialization
using GtkFileChooserNativeUPtr = std::unique_ptr<GtkFileChooserNative, specialization::GtkFileChooserNativeDeleter>;


};  // namespace raii
};  // namespace xoj::util
