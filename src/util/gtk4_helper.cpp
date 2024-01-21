#include "util/gtk4_helper.h"

#include <gtk/gtk.h>

#include "util/Assert.h"
#include "util/GtkUtil.h"
#include "util/Stacktrace.h"

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

/**** GdkEvent ****/

gboolean gdk_event_get_position(GdkEvent* event, double* x, double* y) { return gdk_event_get_coords(event, x, y); }

/**** GtkWindow ****/

void gtk_window_destroy(GtkWindow* win) { gtk_widget_destroy(GTK_WIDGET(win)); }

/**** GtkWidget ****/

void gtk_widget_add_css_class(GtkWidget* widget, const char* css_class) {
    gtk_style_context_add_class(gtk_widget_get_style_context(widget), css_class);
}

void gtk_widget_remove_css_class(GtkWidget* widget, const char* css_class) {
    gtk_style_context_remove_class(gtk_widget_get_style_context(widget), css_class);
}

int gtk_widget_get_width(GtkWidget* widget) { return gtk_widget_get_allocated_width(widget); }

int gtk_widget_get_height(GtkWidget* widget) { return gtk_widget_get_allocated_height(widget); }

gboolean gtk_widget_compute_point(GtkWidget* widget, GtkWidget* target, const graphene_point_t* point,
                                  graphene_point_t* out_point) {
    int offset_x, offset_y;

    bool result = gtk_widget_translate_coordinates(widget, target, 0, 0, &offset_x, &offset_y);

    out_point->x = point->x + static_cast<float>(offset_x);
    out_point->y = point->y + static_cast<float>(offset_y);

    return result;
}

void gtk_widget_add_controller(GtkWidget* widget, GtkEventController* eventController) {
    // Nothing to do here
    // In GTK3, widget is already added through the constructor.
    // This is a placeholder for GTK4
}

/**** GtkEventController ****/

GtkGesture* gtk_gesture_click_new(GtkWidget* widget) { return gtk_gesture_multi_press_new(widget); }

GdkEvent* gtk_event_controller_get_current_event(GtkEventController* controller) { return gtk_get_current_event(); }

gboolean gtk_event_controller_motion_contains_pointer(GtkEventControllerMotion* self) {
    GtkEventController* controller = GTK_EVENT_CONTROLLER(self);
    GtkWidget* widget = gtk_event_controller_get_widget(controller);
    return xoj::util::gtk::isEventOverWidget(controller, widget);
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
