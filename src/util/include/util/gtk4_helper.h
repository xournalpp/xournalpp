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

void gtk_widget_add_css_class(GtkWidget* widget, const char* css_class);
void gtk_widget_remove_css_class(GtkWidget* widget, const char* css_class);

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
