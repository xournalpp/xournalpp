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
/**
 * @brief if `btn` has the GtkActionable properties action-name and action-target set, and a corresponding action has
 * been added to any ascendent's GActionMap, then make the ToggleButton to never be unset when clicking on it. Only
 * changing the action state can unset the button (e.g. by clicking on another button with the same action but a
 * different state).
 */
void setToggleButtonUnreleasable(GtkToggleButton* btn);
};  // namespace xoj::util::gtk
