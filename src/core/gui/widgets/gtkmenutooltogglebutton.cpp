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

//#undef GTK_DISABLE_DEPRECATED /* GtkTooltips */

#include "gtkmenutooltogglebutton.h"

#include <algorithm>  // for max

#include <gdk/gdk.h>           // for GdkRectangle, gdk_window_get_origin
#include <gobject/gmarshal.h>  // for g_cclosure_marshal_VOID__VOID

#define GTK_MENU_TOOL_TOGGLE_BUTTON_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE((object), GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, GtkMenuToolToggleButtonPrivate))

struct _GtkMenuToolToggleButtonPrivate {
    GtkWidget* button;
    GtkWidget* arrow;
    GtkWidget* arrow_button;
    GtkWidget* box;
    GtkMenu* menu;
};

static void gtk_menu_tool_toggle_button_dispose(GObject* object);

static auto menu_deactivate_cb(GtkMenuShell* menu_shell, GtkMenuToolToggleButton* button) -> int;

enum { SHOW_MENU, LAST_SIGNAL };

enum { PROP_0, PROP_MENU };

static gint signals[LAST_SIGNAL];

G_DEFINE_TYPE(GtkMenuToolToggleButton, gtk_menu_tool_toggle_button,  // @suppress("Unused static function")
              GTK_TYPE_TOGGLE_TOOL_BUTTON)

static void gtk_menu_tool_toggle_button_construct_contents(GtkMenuToolToggleButton* button) {
    GtkMenuToolToggleButtonPrivate* priv = button->priv;

    GtkOrientation orientation = gtk_tool_item_get_orientation(GTK_TOOL_ITEM(button));

    GtkWidget* box = nullptr;
    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_image_set_from_icon_name(GTK_IMAGE(priv->arrow), "pan-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    } else {
        box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_image_set_from_icon_name(GTK_IMAGE(priv->arrow), "pan-end-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    }

    if (priv->button && gtk_widget_get_parent(priv->button)) {
        g_object_ref(priv->button);
        gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(priv->button)), priv->button);
        gtk_container_add(GTK_CONTAINER(box), priv->button);
        g_object_unref(priv->button);
    }

    if (priv->arrow_button && gtk_widget_get_parent(priv->arrow_button)) {
        g_object_ref(priv->arrow_button);
        gtk_container_remove(GTK_CONTAINER(gtk_widget_get_parent(priv->arrow_button)), priv->arrow_button);
        gtk_box_pack_end(GTK_BOX(box), priv->arrow_button, false, false, 0);
        g_object_unref(priv->arrow_button);
    }

    if (priv->box) {
        gchar* tmp = nullptr;

        /* Transfer a possible tooltip to the new box */
        g_object_get(priv->box, "tooltip-markup", &tmp, nullptr);

        if (tmp) {
            g_object_set(box, "tooltip-markup", tmp, nullptr);
            g_free(tmp);
        }

        /* Note: we are not destroying the button and the arrow_button
         * here because they were removed from their container above
         */
        gtk_widget_destroy(priv->box);
    }

    priv->box = box;

    gtk_container_add(GTK_CONTAINER(button), priv->box);
    gtk_widget_show_all(priv->box);

    gtk_button_set_relief(GTK_BUTTON(priv->arrow_button), gtk_tool_item_get_relief_style(GTK_TOOL_ITEM(button)));

    gtk_widget_queue_resize(GTK_WIDGET(button));
}

static void gtk_menu_tool_toggle_button_toolbar_reconfigured(GtkToolItem* toolitem) {
    gtk_menu_tool_toggle_button_construct_contents(GTK_MENU_TOOL_TOGGLE_BUTTON(toolitem));

    /* chain up */
    GTK_TOOL_ITEM_CLASS(gtk_menu_tool_toggle_button_parent_class)->toolbar_reconfigured(toolitem);
}

static void gtk_menu_tool_toggle_button_state_changed(GtkWidget* widget, GtkStateType previous_state) {
    GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(widget);
    GtkMenuToolToggleButtonPrivate* priv = button->priv;

    if (!gtk_widget_is_sensitive(widget) && priv->menu) {
        gtk_menu_shell_deactivate(GTK_MENU_SHELL(priv->menu));
    }
}

static void gtk_menu_tool_toggle_button_set_property(GObject* object, guint prop_id, const GValue* value,
                                                     GParamSpec* pspec) {
    GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(object);

    switch (prop_id) {
        case PROP_MENU:
            gtk_menu_tool_toggle_button_set_menu(button, GTK_WIDGET(g_value_get_object(value)));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gtk_menu_tool_toggle_button_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
    GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(object);

    switch (prop_id) {
        case PROP_MENU:
            g_value_set_object(value, button->priv->menu);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void gtk_menu_tool_toggle_button_class_init(GtkMenuToolToggleButtonClass* klass) {
    auto* object_class = reinterpret_cast<GObjectClass*>(klass);
    auto* widget_class = reinterpret_cast<GtkWidgetClass*>(klass);
    auto* toolitem_class = reinterpret_cast<GtkToolItemClass*>(klass);

    object_class->set_property = gtk_menu_tool_toggle_button_set_property;
    object_class->get_property = gtk_menu_tool_toggle_button_get_property;
    object_class->dispose = gtk_menu_tool_toggle_button_dispose;
    widget_class->state_changed = gtk_menu_tool_toggle_button_state_changed;
    toolitem_class->toolbar_reconfigured = gtk_menu_tool_toggle_button_toolbar_reconfigured;

    /**
     * GtkMenuToolButton::show-menu:
     * @button: the object on which the signal is emitted
     *
     * The ::show-menu signal is emitted before the menu is shown.
     *
     * It can be used to populate the menu on demand, using
     * gtk_menu_tool_button_get_menu().

     * Note that even if you populate the menu dynamically in this way,
     * you must set an empty menu on the #GtkMenuToolButton beforehand,
     * since the arrow is made insensitive if the menu is not set.
     */
    signals[SHOW_MENU] = g_signal_new("show-menu", G_OBJECT_CLASS_TYPE(klass), G_SIGNAL_RUN_FIRST,
                                      G_STRUCT_OFFSET(GtkMenuToolToggleButtonClass, show_menu), nullptr, nullptr,
                                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

    GParamSpec* pspec = g_param_spec_object("menu", "Menu", "The dropdown menu", GTK_TYPE_MENU,
                                            static_cast<GParamFlags>(G_PARAM_READWRITE | G_PARAM_STATIC_NAME |
                                                                     G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB));
    g_object_class_install_property(object_class, PROP_MENU, pspec);

    g_type_class_add_private(object_class, sizeof(GtkMenuToolToggleButtonPrivate));
}

static void menu_position_func(GtkMenu* menu, int* x, int* y, gboolean* push_in, GtkMenuToolToggleButton* button) {
    GtkMenuToolToggleButtonPrivate* priv = button->priv;
    GtkWidget* widget = GTK_WIDGET(button);

    GtkRequisition minimum_size;
    GtkRequisition menu_req;
    gtk_widget_get_preferred_size(GTK_WIDGET(priv->menu), &minimum_size, &menu_req);

    GtkOrientation orientation = gtk_tool_item_get_orientation(GTK_TOOL_ITEM(button));
    GtkTextDirection direction = gtk_widget_get_direction(widget);

    auto* display = gtk_widget_get_display(GTK_WIDGET(menu));
    GdkMonitor* monitor = gdk_display_get_monitor_at_window(display, gtk_widget_get_window(widget));
    GdkRectangle monitor_rect;
    gdk_monitor_get_geometry(monitor, &monitor_rect);

    GtkAllocation arrow_allocation;
    gtk_widget_get_allocation(priv->arrow_button, &arrow_allocation);

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);

        gdk_window_get_origin(gtk_widget_get_window(widget), x, y);
        *x += allocation.x;
        *y += allocation.y;

        if (direction == GTK_TEXT_DIR_LTR) {
            *x += std::max(allocation.width - menu_req.width, 0);
        } else if (menu_req.width > allocation.width) {
            *x -= menu_req.width - allocation.width;
        }

        if ((*y + arrow_allocation.height + menu_req.height) <= monitor_rect.y + monitor_rect.height) {
            *y += arrow_allocation.height;
        } else if ((*y - menu_req.height) >= monitor_rect.y) {
            *y -= menu_req.height;
        } else if (monitor_rect.y + monitor_rect.height - (*y + arrow_allocation.height) > *y) {
            *y += arrow_allocation.height;
        } else {
            *y -= menu_req.height;
        }
    } else {
        gdk_window_get_origin(gtk_button_get_event_window(GTK_BUTTON(priv->arrow_button)), x, y);

        GtkRequisition req;
        gtk_widget_get_preferred_size(priv->arrow_button, &minimum_size, &req);

        if (direction == GTK_TEXT_DIR_LTR) {
            *x += arrow_allocation.width;
        } else {
            *x -= menu_req.width;
        }

        if (*y + menu_req.height > monitor_rect.y + monitor_rect.height &&
            *y + arrow_allocation.height - monitor_rect.y > monitor_rect.y + monitor_rect.height - *y) {
            *y += arrow_allocation.height - menu_req.height;
        }
    }

    *push_in = false;
}

static void popup_menu_under_arrow(GtkMenuToolToggleButton* button, GdkEventButton* event) {
    GtkMenuToolToggleButtonPrivate* priv = button->priv;

    g_signal_emit(button, signals[SHOW_MENU], 0);

    if (!priv->menu) {
        return;
    }

    gtk_menu_popup(priv->menu, nullptr, nullptr, reinterpret_cast<GtkMenuPositionFunc>(menu_position_func), button,
                   event ? event->button : 0, event ? event->time : gtk_get_current_event_time());
}

static void arrow_button_toggled_cb(GtkToggleButton* togglebutton, GtkMenuToolToggleButton* button) {
    GtkMenuToolToggleButtonPrivate* priv = button->priv;

    if (!priv->menu) {
        return;
    }

    if (gtk_toggle_button_get_active(togglebutton) && gtk_widget_get_visible(GTK_WIDGET(priv->menu))) {
        /* we get here only when the menu is activated by a key
         * press, so that we can select the first menu item */
        popup_menu_under_arrow(button, nullptr);
        gtk_menu_shell_select_first(GTK_MENU_SHELL(priv->menu), false);
    }
}

static auto arrow_button_button_press_event_cb(GtkWidget* widget, GdkEventButton* event,
                                               GtkMenuToolToggleButton* button) -> gboolean {
    if (event->button == 1) {
        popup_menu_under_arrow(button, event);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), true);

        return true;
    }


    return false;
}

static void gtk_menu_tool_toggle_button_init(GtkMenuToolToggleButton* button) {
    button->priv = GTK_MENU_TOOL_TOGGLE_BUTTON_GET_PRIVATE(button);

    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(button), false);

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget* real_button = gtk_bin_get_child(GTK_BIN(button));
    g_object_ref(real_button);
    gtk_container_remove(GTK_CONTAINER(button), real_button);
    gtk_container_add(GTK_CONTAINER(box), real_button);
    g_object_unref(real_button);

    GtkWidget* arrow_button = gtk_toggle_button_new();
    GtkWidget* arrow = gtk_image_new_from_icon_name("pan-down-symbolic", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_container_add(GTK_CONTAINER(arrow_button), arrow);
    gtk_box_pack_end(GTK_BOX(box), arrow_button, false, false, 0);

    /* the arrow button is insentive until we set a menu */
    gtk_widget_set_sensitive(arrow_button, false);

    gtk_widget_show_all(box);

    gtk_container_add(GTK_CONTAINER(button), box);

    button->priv->button = real_button;
    button->priv->arrow = arrow;
    button->priv->arrow_button = arrow_button;
    button->priv->box = box;

    g_signal_connect(arrow_button, "toggled", G_CALLBACK(arrow_button_toggled_cb), button);
    g_signal_connect(arrow_button, "button-press-event", G_CALLBACK(arrow_button_button_press_event_cb), button);
}

static void gtk_menu_tool_toggle_button_dispose(GObject* object) {
    GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(object);

    if (button->priv->menu) {
        g_signal_handlers_disconnect_by_func(button->priv->menu, (gpointer)G_CALLBACK(menu_deactivate_cb), button);
        gtk_menu_detach(button->priv->menu);

        g_signal_handlers_disconnect_by_func(button->priv->arrow_button, (gpointer)G_CALLBACK(arrow_button_toggled_cb),
                                             button);
        g_signal_handlers_disconnect_by_func(button->priv->arrow_button,
                                             (gpointer)G_CALLBACK(arrow_button_button_press_event_cb), button);
    }
}

/**
 * gtk_menu_tool_button_new:
 * @icon_widget: a widget that will be used as icon widget, or %nullptr
 * @label: a string that will be used as label, or %nullptr
 *
 * Creates a new #GtkMenuToolButton using @icon_widget as icon and
 * @label as label.
 *
 * Return value: the new #GtkMenuToolButton
 *
 * Since: 2.6
 **/
auto gtk_menu_tool_toggle_button_new(GtkWidget* icon_widget, const gchar* label) -> GtkToolItem* {
    void* button = g_object_new(GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, nullptr);

    if (label) {
        gtk_tool_button_set_label(GTK_TOOL_BUTTON(button), label);
    }

    if (icon_widget) {
        gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button), icon_widget);
    }

    return GTK_TOOL_ITEM(button);
}

/**
 * gtk_menu_tool_button_new_from_stock:
 * @stock_id: the name of a stock item
 *
 * Creates a new #GtkMenuToolButton.
 * The new #GtkMenuToolButton will contain an icon and label from
 * the stock item indicated by @stock_id.
 *
 * Return value: the new #GtkMenuToolButton
 *
 * Since: 2.6
 **/
auto gtk_menu_tool_toggle_button_new_from_stock(const gchar* stock_id) -> GtkToolItem* {
    g_return_val_if_fail(stock_id != nullptr, nullptr);

    void* button = g_object_new(GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, "stock-id", stock_id, nullptr);

    return GTK_TOOL_ITEM(button);
}

/* Callback for the "deactivate" signal on the pop-up menu.
 * This is used so that we unset the state of the toggle button
 * when the pop-up menu disappears.
 */
static auto menu_deactivate_cb(GtkMenuShell* menu_shell, GtkMenuToolToggleButton* button) -> int {
    GtkMenuToolToggleButtonPrivate* priv = button->priv;

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->arrow_button), false);

    return true;
}

static void menu_detacher(GtkWidget* widget, GtkMenu* menu) {
    GtkMenuToolToggleButtonPrivate* priv = GTK_MENU_TOOL_TOGGLE_BUTTON(widget)->priv;

    g_return_if_fail(priv->menu == menu);

    priv->menu = nullptr;
}

/**
 * gtk_menu_tool_button_set_menu:
 * @button: a #GtkMenuToolButton
 * @menu: the #GtkMenu associated with #GtkMenuToolButton
 *
 * Sets the #GtkMenu that is popped up when the user clicks on the arrow.
 * If @menu is nullptr, the arrow button becomes insensitive.
 *
 * Since: 2.6
 **/
void gtk_menu_tool_toggle_button_set_menu(GtkMenuToolToggleButton* button, GtkWidget* menu) {
    GtkMenuToolToggleButtonPrivate* priv = nullptr;

    g_return_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button));
    g_return_if_fail(GTK_IS_MENU(menu) || menu == nullptr);

    priv = button->priv;

    if (priv->menu != GTK_MENU(menu)) {
        if (priv->menu && gtk_widget_get_visible(GTK_WIDGET(priv->menu))) {
            gtk_menu_shell_deactivate(GTK_MENU_SHELL(priv->menu));
        }

        if (priv->menu) {
            g_signal_handlers_disconnect_by_func(priv->menu, (gpointer)G_CALLBACK(menu_deactivate_cb), button);
            gtk_menu_detach(priv->menu);
        }

        priv->menu = GTK_MENU(menu);

        if (priv->menu) {
            gtk_menu_attach_to_widget(priv->menu, GTK_WIDGET(button), menu_detacher);

            gtk_widget_set_sensitive(priv->arrow_button, true);

            g_signal_connect(priv->menu, "deactivate", G_CALLBACK(menu_deactivate_cb), button);
        } else {
            gtk_widget_set_sensitive(priv->arrow_button, false);
        }
    }

    g_object_notify(G_OBJECT(button), "menu");
}

/**
 * gtk_menu_tool_button_get_menu:
 * @button: a #GtkMenuToolButton
 *
 * Gets the #GtkMenu associated with #GtkMenuToolButton.
 *
 * Return value: the #GtkMenu associated with #GtkMenuToolButton
 *
 * Since: 2.6
 **/
auto gtk_menu_tool_toggle_button_get_menu(GtkMenuToolToggleButton* button) -> GtkWidget* {
    g_return_val_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button), nullptr);

    return GTK_WIDGET(button->priv->menu);
}

/**
 * gtk_menu_tool_button_set_arrow_tooltip_text:
 * @button: a #GtkMenuToolButton
 * @text: text to be used as tooltip text for button's arrow button
 *
 * Sets the tooltip text to be used as tooltip for the arrow button which
 * pops up the menu.  See gtk_tool_item_set_tooltip() for setting a tooltip
 * on the whole #GtkMenuToolButton.
 *
 * Since: 2.12
 **/
void gtk_menu_tool_toggle_button_set_arrow_tooltip_text(GtkMenuToolToggleButton* button, const gchar* text) {
    g_return_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button));

    gtk_widget_set_tooltip_text(button->priv->arrow_button, text);
}

/**
 * gtk_menu_tool_button_set_arrow_tooltip_markup:
 * @button: a #GtkMenuToolButton
 * @markup: markup text to be used as tooltip text for button's arrow button
 *
 * Sets the tooltip markup text to be used as tooltip for the arrow button
 * which pops up the menu.  See gtk_tool_item_set_tooltip() for setting a
 * tooltip on the whole #GtkMenuToolButton.
 *
 * Since: 2.12
 **/
void gtk_menu_tool_toggle_button_set_arrow_tooltip_markup(GtkMenuToolToggleButton* button, const gchar* markup) {
    g_return_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button));

    gtk_widget_set_tooltip_markup(button->priv->arrow_button, markup);
}
