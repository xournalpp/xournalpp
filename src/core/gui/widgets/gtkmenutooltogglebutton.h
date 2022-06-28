/* GTK - The GIMP Toolkit
 *
 * Copyright (C) 2003 Ricardo Fernandez Pascual
 * Copyright (C) 2004 Paolo Borelli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#pragma once

/**
 * Adapted MenuToolButton to a MenuToolTooggleButton
 */

#ifndef __GTK_MENU_TOOL_TOGGLE_BUTTON_H__
#define __GTK_MENU_TOOL_TOGGLE_BUTTON_H__

#include <glib-object.h>  // for G_TYPE_CHECK_INSTANCE_CAST, G_TYPE_CHECK_IN...
#include <glib.h>         // for gchar, G_BEGIN_DECLS, G_END_DECLS, G_GNUC_C...
#include <gtk/gtk.h>      // for GtkWidget, GtkToolItem, GtkMenuToolButton

struct _GtkMenuToolToggleButton;
struct _GtkMenuToolToggleButtonClass;

G_BEGIN_DECLS

#define GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON (gtk_menu_tool_toggle_button_get_type())
#define GTK_MENU_TOOL_TOGGLE_BUTTON(o) \
    (G_TYPE_CHECK_INSTANCE_CAST((o), GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, GtkMenuToolToggleButton))
#define GTK_MENU_TOOL_TOGGLE_BUTTON_CLASS(k) \
    (G_TYPE_CHECK_CLASS_CAST((k), GTK_TYPE_MENU_TOO_TOGGLEL_BUTTON, GtkMenuToolToggleButtonClass))
#define GTK_IS_MENU_TOOL_TOGGLE_BUTTON(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON))
#define GTK_IS_MENU_TOOL_TOGGLE_BUTTON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON))
#define GTK_MENU_TOOL_TOGGLE_BUTTON_GET_CLASS(o) \
    (G_TYPE_INSTANCE_GET_CLASS((o), GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, GtkMenuToolToggleButtonClass))

typedef struct _GtkMenuToolToggleButtonClass GtkMenuToolToggleButtonClass;
typedef struct _GtkMenuToolToggleButton GtkMenuToolToggleButton;
typedef struct _GtkMenuToolToggleButtonPrivate GtkMenuToolToggleButtonPrivate;

struct _GtkMenuToolToggleButton {
    GtkToggleToolButton parent;

    /*< private >*/
    GtkMenuToolToggleButtonPrivate* priv;
};

struct _GtkMenuToolToggleButtonClass {
    GtkToggleToolButtonClass parent_class;

    void (*show_menu)(GtkMenuToolButton* button);

    /* Padding for future expansion */
    void (*_gtk_reserved1)(void);
    void (*_gtk_reserved2)(void);
    void (*_gtk_reserved3)(void);
    void (*_gtk_reserved4)(void);
};

GType gtk_menu_tool_toggle_button_get_type(void) G_GNUC_CONST;
GtkToolItem* gtk_menu_tool_toggle_button_new(GtkWidget* icon_widget, const gchar* label);
GtkToolItem* gtk_menu_tool_toggle_button_new_from_stock(const gchar* stock_id);

void gtk_menu_tool_toggle_button_set_menu(GtkMenuToolToggleButton* button, GtkWidget* menu);
GtkWidget* gtk_menu_tool_toggle_button_get_menu(GtkMenuToolToggleButton* button);

void gtk_menu_tool_toggle_button_set_arrow_tooltip_text(GtkMenuToolToggleButton* button, const gchar* text);
void gtk_menu_tool_toggle_button_set_arrow_tooltip_markup(GtkMenuToolToggleButton* button, const gchar* markup);

G_END_DECLS

#endif /* __GTK_MENU_TOOL_TOGGLE_BUTTON_H__ */
