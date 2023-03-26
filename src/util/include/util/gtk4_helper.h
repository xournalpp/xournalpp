/*
 * Xournal++
 *
 * header for missing gtk4 functions (part of the gtk4 port)
 * will be removed later
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>


inline void gtk_box_append(GtkBox* box, GtkWidget* child) {
    constexpr auto default_expand = false;
    gtk_box_pack_start(GTK_BOX(box), child, default_expand, true, 0);
}

inline void gtk_box_remove(GtkBox* box, GtkWidget* child) { gtk_container_remove(GTK_CONTAINER(box), child); }

inline void gtk_window_destroy(GtkWindow* win) { gtk_widget_destroy(GTK_WIDGET(win)); }

inline bool gtk_check_button_get_active(GtkCheckButton* bt) {
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bt));
}
inline void gtk_check_button_set_active(GtkCheckButton* bt, bool state) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bt), state);
}

inline void gtk_widget_add_css_class(GtkWidget* widget, const char* css_class) {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), css_class);
}

inline void gtk_widget_remove_css_class(GtkWidget* widget, const char* css_class) {
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), css_class);
}
