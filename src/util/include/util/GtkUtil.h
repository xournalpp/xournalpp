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

#include <gtk/gtk.h>

namespace xoj::util::gtk {

// Margins relative to widget borders
struct WidgetMargins {
    double top;
    double right;
    double bottom;
    double left;
};

/**
 * @brief Returns true if event is over widget.
 * Margins will redefine the widget area.
 *
 * `margins.left = -1`, the device is still over the widget when 1px left of left border.
 *
 * `margins.right = 0`, the device is still over the widget when exactly on the right border.
 * `margins.right = -1`, the device is no longer over the widget when exactly on the right border.
 *
 * `margins.right = 0` signifies the device is still over the widget when 1px right of right border.
 */
bool isEventOverWidget(GtkEventController*, GtkWidget* widget, WidgetMargins margins);

/**
 * Same as isEventOverWidget(GtkEventController*, GtkWidget* widget, {0,0,0,0})
 */
bool isEventOverWidget(GtkEventController* eventController, GtkWidget* widget);

/**
 * @brief if `btn` has the GtkActionable properties action-name and action-target set, and a corresponding action has
 * been added to any ascendent's GActionMap, then make the ToggleButton to never be unset when clicking on it. Only
 * changing the action state can unset the button (e.g. by clicking on another button with the same action but a
 * different state).
 */
void setToggleButtonUnreleasable(GtkToggleButton* btn);

/**
 * @brief Make so a widget is automatically enabled/disabled whenever the given action is
 */
void setWidgetFollowActionEnabled(GtkWidget* w, GAction* a);

#if GTK_MAJOR_VERSION == 3
/**
 * @brief RadioButton's and GAction don't work as expected in GTK3:
 * * Without setting the group, the RadioButtons are simply always ticked (all of them)
 * * With the group properly set, when a RadioButton is "un-selected" (because we selected another one), it still
 *   changes the GAction's state (it should only do that when being selected). This leads to infinite callback loops
 *
 * To circumvent this, we make our own GAction/RadioButton interactions. Don't forget to group the buttons together.
 */
void setRadioButtonActionName(GtkRadioButton* btn, const char* actionNamespace, const char* actionName);

/**
 * @brief some Gtk3 GtkActionable implementations (e.g. GtkMEnuItem) do not set their sensitivity properly on startup
 * Fixes that
 */
void fixActionableInitialSensitivity(GtkActionable* w);
#endif
};  // namespace xoj::util::gtk
