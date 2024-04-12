/*
 * Xournal++
 * Helper function for setting up some GtkWidget
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <optional>

#include <gtk/gtk.h>

namespace xoj::util::gtk {
/**
 * @brief Make so a widget is automatically enabled/disabled whenever the given action is
 */
void setWidgetFollowActionEnabled(GtkWidget* w, GAction* a);

/**
 * @brief returns the user-space DPI (i.e. scaled by HiDPI scaling) of the monitor the widget is on.
 * Return std::nullopt if the widget is not realized
 */
std::optional<double> getWidgetDPI(GtkWidget* w);
};  // namespace xoj::util::gtk
