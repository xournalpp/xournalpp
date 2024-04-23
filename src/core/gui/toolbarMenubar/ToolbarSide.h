/*
 * Xournal++
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/safe_casts.h"

enum class ToolbarSide {  // Do not change the order!
    TOP = 0,
    START,  ///< left in left-to-right languages, right in right-to-left languages
    BOTTOM,
    END,  ///< right in left-to-right languages, left in right-to-left languages
};

static inline constexpr GtkOrientation to_Orientation(ToolbarSide s) {
    return GtkOrientation(xoj::to_underlying(s) % 2);
}
static_assert(to_Orientation(ToolbarSide::TOP) == GTK_ORIENTATION_HORIZONTAL);
static_assert(to_Orientation(ToolbarSide::BOTTOM) == GTK_ORIENTATION_HORIZONTAL);
static_assert(to_Orientation(ToolbarSide::END) == GTK_ORIENTATION_VERTICAL);
static_assert(to_Orientation(ToolbarSide::START) == GTK_ORIENTATION_VERTICAL);

static inline constexpr GtkArrowType to_ArrowType(ToolbarSide s, bool rightToLeft) {
    switch (s) {
        case ToolbarSide::TOP:
            return GTK_ARROW_DOWN;
        case ToolbarSide::BOTTOM:
            return GTK_ARROW_UP;
        case ToolbarSide::START:
            return rightToLeft ? GTK_ARROW_RIGHT : GTK_ARROW_LEFT;
        case ToolbarSide::END:
            return rightToLeft ? GTK_ARROW_LEFT : GTK_ARROW_RIGHT;
        default:
            xoj_assert(false);
            return GTK_ARROW_NONE;
    }
}

static inline void setMenuButtonDirection(GtkMenuButton* btn, ToolbarSide s) {
    gtk_menu_button_set_direction(btn, to_ArrowType(s, gtk_widget_get_direction(GTK_WIDGET(btn)) == GTK_TEXT_DIR_RTL));
}
