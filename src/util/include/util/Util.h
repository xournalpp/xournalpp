/*
 * Xournal++
 *
 * Xournal util functions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstdlib>     // size_t
#include <functional>  // for function
#include <limits>      // for numeric_limits
#include <utility>

#include <cairo.h>    // for cairo_t
#include <glib.h>     // for G_PRIORITY_DEFAULT_IDLE, gboolean, gchar, gint
#include <gtk/gtk.h>  // for GtkWidget

#include "util/glib_casts.h"

#include "Point.h"

class OutputStream;

namespace Util {

#if defined(_MSC_VER)
using PID = uint32_t;  // DWORD
#else
using PID = int32_t;  // pid
#endif

auto getPid() -> PID;

/**
 * Wrap the system call to redirect errors to a dialog
 */
void systemWithMessage(const char* command);

/**
 * Check if currently running in a Flatpak sandbox
 */
bool isFlatpakInstallation();

/**
 * Execute the callback in the UI Thread.
 *
 * Make sure the container class is not deleted before the UI stuff is finished!
 */
template <typename Fun>
void execInUiThread(Fun&& callback, gint priority = G_PRIORITY_DEFAULT_IDLE) {
    if constexpr (std::is_function_v<Fun>) {
        gdk_threads_add_idle_full(priority, std::forward<Fun>(callback), nullptr, nullptr);
    } else {
        constexpr auto fn = +[](gpointer functor) -> int {
            auto fun = static_cast<Fun*>(functor);
            (*fun)();
            return G_SOURCE_REMOVE;
        };
        gdk_threads_add_idle_full(priority, fn, new auto(std::forward<Fun>(callback)), &xoj::util::destroy_cb<Fun>);
    }
}

gboolean paintBackgroundWhite(GtkWidget* widget, cairo_t* cr, void* unused);

void cairo_set_dash_from_vector(cairo_t* cr, const std::vector<double>& dashes, double offset);

/**
 * Transform absolute coordinates into coordinates local to the specified widget.
 * The top left corner of `widget` will have coordinates (0, 0).
 */
xoj::util::Point<double> toWidgetCoords(GtkWidget* widget, xoj::util::Point<double> absolute_coords);

/**
 * Format coordinates to use 8 digits of precision https://m.xkcd.com/2170/
 * This function directly writes to the given OutputStream.
 */
extern void writeCoordinateString(OutputStream* out, double xVal, double yVal);

constexpr const gchar* PRECISION_FORMAT_STRING = "%.8g";

constexpr const auto DPI_NORMALIZATION_FACTOR = 72.0;

}  // namespace Util

constexpr auto npos = std::numeric_limits<size_t>::max();
