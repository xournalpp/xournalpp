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

/**** GtkBox ****/

void gtk_box_append(GtkBox* box, GtkWidget* child);
void gtk_box_remove(GtkBox* box, GtkWidget* child);

/**** GtkWindow ****/

void gtk_window_destroy(GtkWindow* win);

/**** GtkWidget ****/

struct graphene_point_t {
    float x;
    float y;
};

void gtk_widget_add_css_class(GtkWidget* widget, const char* css_class);
void gtk_widget_remove_css_class(GtkWidget* widget, const char* css_class);
int gtk_widget_get_width(GtkWidget* widget);
int gtk_widget_get_height(GtkWidget* widget);
gboolean gtk_widget_compute_point(GtkWidget* widget, GtkWidget* target, const graphene_point_t* point,
                                  graphene_point_t* out_point);

/**** GtkEvent ****/

gboolean gdk_event_get_position(GdkEvent* event, double* x, double* y);

/**** GtkEventController ****/

GtkGesture* gtk_gesture_click_new(GtkWidget* widget);

void gtk_widget_add_controller(GtkWidget* widget, GtkEventController* controller);

GdkEvent* gtk_event_controller_get_current_event(GtkEventController* controller);

gboolean gtk_event_controller_motion_contains_pointer(GtkEventControllerMotion* self);

/*** GtkDrawingArea ****/

typedef void (*GtkDrawingAreaDrawFunc)(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height,
                                       gpointer user_data);
/// WARNING: unsetting (via draw_func = nullptr) or replacing the function is not implemented here.
void gtk_drawing_area_set_draw_func(GtkDrawingArea* area, GtkDrawingAreaDrawFunc draw_func, gpointer user_data,
                                    GDestroyNotify destroy);

/**** GtkScrolledWindow ****/

void gtk_scrolled_window_set_child(GtkScrolledWindow* win, GtkWidget* child);
GtkWidget* gtk_scrolled_window_get_child(GtkScrolledWindow* win);

/**** GtkCheckButton ****/

void gtk_check_button_set_child(GtkCheckButton* button, GtkWidget* child);
void gtk_check_button_set_label(GtkCheckButton* button, const char* label);

bool gtk_check_button_get_active(GtkCheckButton* bt);
void gtk_check_button_set_active(GtkCheckButton* bt, bool state);

/**** GtkButton ****/

void gtk_button_set_child(GtkButton* button, GtkWidget* child);
GtkWidget* gtk_button_get_child(GtkButton* button);
void gtk_button_set_icon_name(GtkButton* button, const char* iconName);

/**** GtkPopover ****/

GtkWidget* gtk_popover_new();
void gtk_popover_set_child(GtkPopover* popover, GtkWidget* child);
GtkWidget* gtk_popover_menu_new_from_model(GMenuModel* model);

/**** GtkLabel ****/
void gtk_label_set_wrap(GtkLabel* label, gboolean wrap);
void gtk_label_set_wrap_mode(GtkLabel* label, PangoWrapMode wrap_mode);
