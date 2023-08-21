#include "util/gtk4_helper.h"

#include <gtk/gtk.h>

#include "util/Assert.h"

namespace {
void set_child(GtkContainer* c, GtkWidget* child) {
    gtk_container_foreach(
            c, +[](GtkWidget* child, gpointer c) { gtk_container_remove(GTK_CONTAINER(c), child); }, c);
    gtk_container_add(c, child);
}
};  // namespace

/**** GtkBox ****/

void gtk_box_append(GtkBox* box, GtkWidget* child) {
    constexpr auto default_expand = false;
    gtk_box_pack_start(GTK_BOX(box), child, default_expand, true, 0);
}

void gtk_box_remove(GtkBox* box, GtkWidget* child) { gtk_container_remove(GTK_CONTAINER(box), child); }

/**** GtkWindow ****/

void gtk_window_destroy(GtkWindow* win) { gtk_widget_destroy(GTK_WIDGET(win)); }

/**** GtkWidget ****/

void gtk_widget_add_css_class(GtkWidget* widget, const char* css_class) {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), css_class);
}

void gtk_widget_remove_css_class(GtkWidget* widget, const char* css_class) {
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), css_class);
}

/*** GtkDrawingArea ****/

void gtk_drawing_area_set_draw_func(GtkDrawingArea* area, GtkDrawingAreaDrawFunc draw_func, gpointer user_data,
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

/**** GtkScrolledWindow ****/

void gtk_scrolled_window_set_child(GtkScrolledWindow* win, GtkWidget* child) { set_child(GTK_CONTAINER(win), child); }
GtkWidget* gtk_scrolled_window_get_child(GtkScrolledWindow* win) { return gtk_bin_get_child(GTK_BIN(win)); }

/**** GtkCheckButton ****/

void gtk_check_button_set_child(GtkCheckButton* button, GtkWidget* child) { set_child(GTK_CONTAINER(button), child); }

void gtk_check_button_set_label(GtkCheckButton* button, const char* label) {
    gtk_check_button_set_child(GTK_CHECK_BUTTON(button), gtk_label_new(label));
}

bool gtk_check_button_get_active(GtkCheckButton* bt) { return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bt)); }

void gtk_check_button_set_active(GtkCheckButton* bt, bool state) {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bt), state);
}

/**** GtkButton ****/

void gtk_button_set_child(GtkButton* button, GtkWidget* child) { set_child(GTK_CONTAINER(button), child); }
GtkWidget* gtk_button_get_child(GtkButton* button) { return gtk_bin_get_child(GTK_BIN(button)); }

/**** GtkPopover ****/

GtkWidget* gtk_popover_new() { return gtk_popover_new(nullptr); }
void gtk_popover_set_child(GtkPopover* popover, GtkWidget* child) { set_child(GTK_CONTAINER(popover), child); }

/**** GtkLabel ****/
void gtk_label_set_wrap(GtkLabel* label, gboolean wrap) { gtk_label_set_line_wrap(label, wrap); }
void gtk_label_set_wrap_mode(GtkLabel* label, PangoWrapMode wrap_mode) {
    gtk_label_set_line_wrap_mode(label, wrap_mode);
}
