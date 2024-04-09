#include "util/gtk4_helper.h"

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/raii/CStringWrapper.h"

#if GTK_MAJOR_VERSION == 3

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

void gtk_box_prepend(GtkBox* box, GtkWidget* child) {
    gtk_box_append(box, child);
    gtk_box_reorder_child(box, child, 0);
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

int gtk_widget_get_width(GtkWidget* widget) {
    GtkAllocation allocation;
    gtk_widget_get_allocation(widget, &allocation);
    if (allocation.width <= 1 && allocation.height <= 1) {
        return 0;
    } else {
        return allocation.width;
    }
}

GtkClipboard* gtk_widget_get_clipboard(GtkWidget* widget) {
    return gtk_widget_get_clipboard(widget, GDK_SELECTION_CLIPBOARD);
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
                          data, GClosureNotify(+[](gpointer d, GClosure*) {
                              auto* data = static_cast<Data*>(d);
                              if (data && data->destroy) {
                                  data->destroy(data->data);
                              }
                              delete data;
                          }),
                          GConnectFlags(0U));  // 0 = G_CONNECT_DEFAULT only introduced in GObject 2.74
}

/**** GtkScale ****/

void gtk_scale_set_format_value_func(GtkScale* scale, GtkScaleFormatValueFunc func, gpointer user_data,
                                     GDestroyNotify destroy_notify) {
    xoj_assert(func != nullptr);
    struct Data {
        gpointer data;
        GtkScaleFormatValueFunc func;
        GDestroyNotify destroy;
    };
    Data* data = new Data{user_data, func, destroy_notify};
    g_signal_connect_data(scale, "format-value", G_CALLBACK(+[](GtkScale* self, gdouble value, gpointer user_data) {
                              auto* data = static_cast<Data*>(user_data);
                              return data->func(self, value, data->data);
                          }),
                          data, GClosureNotify(+[](gpointer d, GClosure*) {
                              auto* data = static_cast<Data*>(d);
                              if (data && data->destroy) {
                                  data->destroy(data->data);
                              }
                              delete data;
                          }),
                          GConnectFlags(0U));  // 0 = G_CONNECT_DEFAULT only introduced in GObject 2.74
}

/**** GtkScrolledWindow ****/

GtkWidget* gtk_scrolled_window_new() { return gtk_scrolled_window_new(nullptr, nullptr); }
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

void gtk_button_set_icon_name(GtkButton* button, const char* iconName) {
    // Is GTK_ICON_SIZE_LARGE_TOOLBAR i the right size??
    gtk_button_set_image(button, gtk_image_new_from_icon_name(iconName, GTK_ICON_SIZE_LARGE_TOOLBAR));
}

/**** GtkPopover ****/

GtkWidget* gtk_popover_new() { return gtk_popover_new(nullptr); }
void gtk_popover_set_child(GtkPopover* popover, GtkWidget* child) { set_child(GTK_CONTAINER(popover), child); }
GtkWidget* gtk_popover_menu_new_from_model(GMenuModel* model) { return gtk_popover_new_from_model(nullptr, model); }

/**** GtkLabel ****/
void gtk_label_set_wrap(GtkLabel* label, gboolean wrap) { gtk_label_set_line_wrap(label, wrap); }
void gtk_label_set_wrap_mode(GtkLabel* label, PangoWrapMode wrap_mode) {
    gtk_label_set_line_wrap_mode(label, wrap_mode);
}

/**** GtkIMContext ****/
void gtk_im_context_set_client_widget(GtkIMContext* context, GtkWidget* widget) {
    gtk_im_context_set_client_window(context, widget ? gtk_widget_get_parent_window(widget) : nullptr);
}

/**** GtkFileChooserDialog ****/
gboolean gtk_file_chooser_add_shortcut_folder(GtkFileChooser* chooser, GFile* file, GError** error) {
    auto uri = xoj::util::OwnedCString::assumeOwnership(g_file_get_uri(file));
    return gtk_file_chooser_add_shortcut_folder(chooser, uri.get(), error);
}
gboolean gtk_file_chooser_set_current_folder(GtkFileChooser* chooser, GFile* file, GError** error) {
    return gtk_file_chooser_set_current_folder_file(chooser, file, error);
}

/**** GtkFixed ****/
void gtk_fixed_remove(GtkFixed* fixed, GtkWidget* child) { gtk_container_remove(GTK_CONTAINER(fixed), child); }

/**** GtkListBox ****/
void gtk_list_box_append(GtkListBox* box, GtkWidget* widget) { gtk_container_add(GTK_CONTAINER(box), widget); }
void gtk_list_box_row_set_child(GtkListBoxRow* row, GtkWidget* w) { set_child(GTK_CONTAINER(row), w); }
GtkWidget* gtk_list_box_row_get_child(GtkListBoxRow* row) { return gtk_bin_get_child(GTK_BIN(row)); }

/**** GtkEventController ****/
GdkEvent* gtk_event_controller_get_current_event(GtkEventController*) { return gtk_get_current_event(); }

/**** Gtk ****/
void gtk_show_uri(GtkWindow* parent, const char* uri, guint32 timestamp) {
    gtk_show_uri_on_window(parent, uri, timestamp, nullptr);
}

#endif
