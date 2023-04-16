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

#include "util/Assert.h"

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

typedef void (*GtkDrawingAreaDrawFunc)(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height,
                                       gpointer user_data);
/// WARNING: unsetting (via draw_func = nullptr) or replacing the function is not implemented here.
inline void gtk_drawing_area_set_draw_func(GtkDrawingArea* area, GtkDrawingAreaDrawFunc draw_func, gpointer user_data,
                                           GDestroyNotify destroy) {
    xoj_assert(draw_func != nullptr);
    struct Data {
        gpointer data;
        GtkDrawingAreaDrawFunc draw_func;
        GDestroyNotify destroy;
    };
    Data* data = new Data{user_data, draw_func, destroy};
    g_signal_connect_data(area, "draw", G_CALLBACK(+[](GtkDrawingArea* self, cairo_t* cr, Data* d) {
                              GtkAllocation alloc;
                              gtk_widget_get_allocation(GTK_WIDGET(self), &alloc);
                              d->draw_func(self, cr, alloc.width, alloc.height, d->data);
                          }),
                          data, GClosureNotify(+[](Data* d, GClosure*) {
                              if (d && d->destroy) {
                                  d->destroy(d->data);
                              }
                              delete d;
                          }),
                          GConnectFlags(0U));  // 0 = G_CONNECT_DEFAULT only introduced in GObject 2.74
}

namespace gtk4_helper {
template <class GtkBinType>
static inline void set_child(GtkBinType* c, GtkWidget* child) {
    gtk_container_foreach(
            GTK_CONTAINER(c), +[](GtkWidget* child, gpointer c) { gtk_container_remove(GTK_CONTAINER(c), child); }, c);
    gtk_container_add(GTK_CONTAINER(c), child);
}
template <class GtkBinType>
static inline GtkWidget* get_child(GtkBinType* c) {
    return gtk_bin_get_child(GTK_BIN(c));
}
};  // namespace gtk4_helper
constexpr auto gtk_scrolled_window_set_child = gtk4_helper::set_child<GtkScrolledWindow>;
constexpr auto gtk_scrolled_window_get_child = gtk4_helper::get_child<GtkScrolledWindow>;
