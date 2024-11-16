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

#if GTK_MAJOR_VERSION == 3

/**** GtkBox ****/

void gtk_box_append(GtkBox* box, GtkWidget* child);
void gtk_box_prepend(GtkBox* box, GtkWidget* child);
void gtk_box_remove(GtkBox* box, GtkWidget* child);

/**** GtkWindow ****/

void gtk_window_destroy(GtkWindow* win);

/**** GtkWidget ****/

void gtk_widget_add_css_class(GtkWidget* widget, const char* css_class);
void gtk_widget_remove_css_class(GtkWidget* widget, const char* css_class);
int gtk_widget_get_width(GtkWidget* widget);
GtkClipboard* gtk_widget_get_clipboard(GtkWidget* widget);

/*** GtkDrawingArea ****/

typedef void (*GtkDrawingAreaDrawFunc)(GtkDrawingArea* drawing_area, cairo_t* cr, int width, int height,
                                       gpointer user_data);
/// WARNING: unsetting (via draw_func = nullptr) or replacing the function is not implemented here.
void gtk_drawing_area_set_draw_func(GtkDrawingArea* area, GtkDrawingAreaDrawFunc draw_func, gpointer user_data,
                                    GDestroyNotify destroy);

/**** GtkScale ****/

typedef char* (*GtkScaleFormatValueFunc)(GtkScale* scale, double value, gpointer user_data);
/// WARNING: unsetting (via func = nullptr) or replacing the function is not implemented here.
void gtk_scale_set_format_value_func(GtkScale* scale, GtkScaleFormatValueFunc func, gpointer user_data,
                                     GDestroyNotify destroy_notify);

/**** GtkScrolledWindow ****/

GtkWidget* gtk_scrolled_window_new();
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

/**** GtkIMContext ****/
void gtk_im_context_set_client_widget(GtkIMContext* context, GtkWidget* widget);

/**** GtkFileChooserDialog ****/
gboolean gtk_file_chooser_add_shortcut_folder(GtkFileChooser* chooser, GFile* file, GError** error);
gboolean gtk_file_chooser_set_current_folder(GtkFileChooser* chooser, GFile* file, GError** error);

/**** GtkFixed ****/
void gtk_fixed_remove(GtkFixed* fixed, GtkWidget* child);

/**** GtkListBox ****/
void gtk_list_box_append(GtkListBox* box, GtkWidget* widget);
void gtk_list_box_row_set_child(GtkListBoxRow* row, GtkWidget* w);
GtkWidget* gtk_list_box_row_get_child(GtkListBoxRow* row);

/**** GtkEventController ****/
GdkEvent* gtk_event_controller_get_current_event(GtkEventController*);

/**** Gtk ****/
void gtk_show_uri(GtkWindow* parent, const char* uri, guint32 timestamp);

#endif
