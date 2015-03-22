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

#undef GTK_DISABLE_DEPRECATED /* GtkTooltips */

#include "gtkmenutooltogglebutton.h"
#include <gtk/gtkprivate.h>

#define GTK_MENU_TOOL_TOGGLE_BUTTON_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, GtkMenuToolToggleButtonPrivate))

struct _GtkMenuToolToggleButtonPrivate
{
	GtkWidget* button;
	GtkWidget* arrow;
	GtkWidget* arrow_button;
	GtkWidget* box;
	GtkMenu* menu;
};

static void gtk_menu_tool_toggle_button_destroy(GtkObject* object);

static int menu_deactivate_cb(GtkMenuShell* menu_shell,
							  GtkMenuToolToggleButton* button);

enum
{
	SHOW_MENU, LAST_SIGNAL
};

enum
{
	PROP_0, PROP_MENU
};

static gint signals[LAST_SIGNAL];

G_DEFINE_TYPE(GtkMenuToolToggleButton, gtk_menu_tool_toggle_button,
			  GTK_TYPE_TOGGLE_TOOL_BUTTON)

static void gtk_menu_tool_toggle_button_construct_contents(
														   GtkMenuToolToggleButton* button)
{
	GtkMenuToolToggleButtonPrivate* priv = button->priv;
	GtkWidget* box;
	GtkOrientation orientation;

	orientation = gtk_tool_item_get_orientation(GTK_TOOL_ITEM(button));

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		box = gtk_hbox_new(FALSE, 0);
		gtk_arrow_set(GTK_ARROW(priv->arrow), GTK_ARROW_DOWN, GTK_SHADOW_NONE);
	}
	else
	{
		box = gtk_vbox_new(FALSE, 0);
		gtk_arrow_set(GTK_ARROW(priv->arrow), GTK_ARROW_RIGHT, GTK_SHADOW_NONE);
	}

	if (priv->button && priv->button->parent)
	{
		g_object_ref(priv->button);
		gtk_container_remove(GTK_CONTAINER(priv->button->parent), priv->button);
		gtk_container_add(GTK_CONTAINER(box), priv->button);
		g_object_unref(priv->button);
	}

	if (priv->arrow_button && priv->arrow_button->parent)
	{
		g_object_ref(priv->arrow_button);
		gtk_container_remove(GTK_CONTAINER(priv->arrow_button->parent),
							priv->arrow_button);
		gtk_box_pack_end(GTK_BOX(box), priv->arrow_button, FALSE, FALSE, 0);
		g_object_unref(priv->arrow_button);
	}

	if (priv->box)
	{
		gchar* tmp;

		/* Transfer a possible tooltip to the new box */
		g_object_get(priv->box, "tooltip-markup", &tmp, NULL);

		if (tmp)
		{
			g_object_set(box, "tooltip-markup", tmp, NULL);
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

	gtk_button_set_relief(GTK_BUTTON(priv->arrow_button),
						gtk_tool_item_get_relief_style(GTK_TOOL_ITEM(button)));

	gtk_widget_queue_resize(GTK_WIDGET(button));
}

static void gtk_menu_tool_toggle_button_toolbar_reconfigured(
															 GtkToolItem* toolitem)
{
	gtk_menu_tool_toggle_button_construct_contents(GTK_MENU_TOOL_TOGGLE_BUTTON(
																			toolitem));

	/* chain up */
	GTK_TOOL_ITEM_CLASS(
						gtk_menu_tool_toggle_button_parent_class)->toolbar_reconfigured(toolitem);
}

static void gtk_menu_tool_toggle_button_state_changed(GtkWidget* widget,
													  GtkStateType previous_state)
{
	GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(widget);
	GtkMenuToolToggleButtonPrivate* priv = button->priv;

	if (!GTK_WIDGET_IS_SENSITIVE(widget) && priv->menu)
	{
		gtk_menu_shell_deactivate(GTK_MENU_SHELL(priv->menu));
	}
}

static void gtk_menu_tool_toggle_button_set_property(GObject* object,
													 guint prop_id, const GValue* value,
													 GParamSpec* pspec)
{
	GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(object);

	switch (prop_id)
	{
	case PROP_MENU:
		gtk_menu_tool_toggle_button_set_menu(button,
											GTK_WIDGET(g_value_get_object(value)));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void gtk_menu_tool_toggle_button_get_property(GObject* object,
													 guint prop_id, GValue* value, GParamSpec* pspec)
{
	GtkMenuToolToggleButton* button = GTK_MENU_TOOL_TOGGLE_BUTTON(object);

	switch (prop_id)
	{
	case PROP_MENU:
		g_value_set_object(value, button->priv->menu);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void gtk_menu_tool_toggle_button_class_init(GtkMenuToolToggleButtonClass*
												   klass)
{
	GObjectClass* object_class;
	GtkObjectClass* gtk_object_class;
	GtkWidgetClass* widget_class;
	GtkToolItemClass* toolitem_class;

	object_class = (GObjectClass*) klass;
	gtk_object_class = (GtkObjectClass*) klass;
	widget_class = (GtkWidgetClass*) klass;
	toolitem_class = (GtkToolItemClass*) klass;

	object_class->set_property = gtk_menu_tool_toggle_button_set_property;
	object_class->get_property = gtk_menu_tool_toggle_button_get_property;
	gtk_object_class->destroy = gtk_menu_tool_toggle_button_destroy;
	widget_class->state_changed = gtk_menu_tool_toggle_button_state_changed;
	toolitem_class->toolbar_reconfigured =
			gtk_menu_tool_toggle_button_toolbar_reconfigured;

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
	signals[SHOW_MENU] = g_signal_new("show-menu", G_OBJECT_CLASS_TYPE(klass),
									G_SIGNAL_RUN_FIRST,
									G_STRUCT_OFFSET(GtkMenuToolToggleButtonClass, show_menu), NULL, NULL,
									g_cclosure_marshal_VOID__VOID,
									G_TYPE_NONE, 0);

	GParamSpec* pspec = g_param_spec_object("menu", "Menu",
											"The dropdown menu", GTK_TYPE_MENU, (GParamFlags) (GTK_PARAM_READWRITE));
	g_object_class_install_property(object_class, PROP_MENU, pspec);

	g_type_class_add_private(object_class, sizeof (GtkMenuToolToggleButtonPrivate));
}

static void menu_position_func(GtkMenu* menu, int* x, int* y, gboolean* push_in,
							   GtkMenuToolToggleButton* button)
{
	GtkMenuToolToggleButtonPrivate* priv = button->priv;
	GtkWidget* widget = GTK_WIDGET(button);
	GtkRequisition req;
	GtkRequisition menu_req;
	GtkOrientation orientation;
	GtkTextDirection direction;
	GdkRectangle monitor;
	gint monitor_num;
	GdkScreen* screen;

	gtk_widget_size_request(GTK_WIDGET(priv->menu), &menu_req);

	orientation = gtk_tool_item_get_orientation(GTK_TOOL_ITEM(button));
	direction = gtk_widget_get_direction(widget);

	screen = gtk_widget_get_screen(GTK_WIDGET(menu));
	monitor_num = gdk_screen_get_monitor_at_window(screen, widget->window);
	if (monitor_num < 0)
		monitor_num = 0;
	gdk_screen_get_monitor_geometry(screen, monitor_num, &monitor);

	if (orientation == GTK_ORIENTATION_HORIZONTAL)
	{
		gdk_window_get_origin(widget->window, x, y);
		*x += widget->allocation.x;
		*y += widget->allocation.y;

		if (direction == GTK_TEXT_DIR_LTR)
			*x += MAX(widget->allocation.width - menu_req.width, 0);
		else if (menu_req.width > widget->allocation.width)
			*x -= menu_req.width - widget->allocation.width;

		if ((*y + priv->arrow_button->allocation.height + menu_req.height) <= monitor.y
			+ monitor.height)
			*y += priv->arrow_button->allocation.height;
		else if ((*y - menu_req.height) >= monitor.y)
			*y -= menu_req.height;
		else if (monitor.y + monitor.height - (*y +
											   priv->arrow_button->allocation.height) > *y)
			*y += priv->arrow_button->allocation.height;
		else
			*y -= menu_req.height;
	}
	else
	{
		gdk_window_get_origin(GTK_BUTTON(priv->arrow_button)->event_window, x, y);
		gtk_widget_size_request(priv->arrow_button, &req);

		if (direction == GTK_TEXT_DIR_LTR)
			*x += priv->arrow_button->allocation.width;
		else
			*x -= menu_req.width;

		if (*y + menu_req.height > monitor.y + monitor.height &&
			*y + priv->arrow_button->allocation.height - monitor.y
			> monitor.y + monitor.height - *y)
			*y += priv->arrow_button->allocation.height - menu_req.height;
	}

	*push_in = FALSE;
}

static void popup_menu_under_arrow(GtkMenuToolToggleButton* button,
								   GdkEventButton* event)
{
	GtkMenuToolToggleButtonPrivate* priv = button->priv;

	g_signal_emit(button, signals[SHOW_MENU], 0);

	if (!priv->menu)
		return;

	gtk_menu_popup(priv->menu, NULL, NULL, (GtkMenuPositionFunc) menu_position_func,
				button, event ? event->button : 0,
				event ? event->time : gtk_get_current_event_time());
}

static void arrow_button_toggled_cb(GtkToggleButton* togglebutton,
									GtkMenuToolToggleButton* button)
{
	GtkMenuToolToggleButtonPrivate* priv = button->priv;

	if (!priv->menu)
		return;

	if (gtk_toggle_button_get_active(togglebutton) &&
		!GTK_WIDGET_VISIBLE(priv->menu))
	{
		/* we get here only when the menu is activated by a key
		 * press, so that we can select the first menu item */
		popup_menu_under_arrow(button, NULL);
		gtk_menu_shell_select_first(GTK_MENU_SHELL(priv->menu), FALSE);
	}
}

static gboolean arrow_button_button_press_event_cb(GtkWidget* widget,
												   GdkEventButton* event,
												   GtkMenuToolToggleButton* button)
{
	if (event->button == 1)
	{
		popup_menu_under_arrow(button, event);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

static void gtk_menu_tool_toggle_button_init(GtkMenuToolToggleButton* button)
{
	GtkWidget* box;
	GtkWidget* arrow;
	GtkWidget* arrow_button;
	GtkWidget* real_button;

	button->priv = GTK_MENU_TOOL_TOGGLE_BUTTON_GET_PRIVATE(button);

	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(button), FALSE);

	box = gtk_hbox_new(FALSE, 0);

	real_button = GTK_BIN(button)->child;
	g_object_ref(real_button);
	gtk_container_remove(GTK_CONTAINER(button), real_button);
	gtk_container_add(GTK_CONTAINER(box), real_button);
	g_object_unref(real_button);

	arrow_button = gtk_toggle_button_new();
	arrow = gtk_arrow_new(GTK_ARROW_DOWN, GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(arrow_button), arrow);
	gtk_box_pack_end(GTK_BOX(box), arrow_button, FALSE, FALSE, 0);

	/* the arrow button is insentive until we set a menu */
	gtk_widget_set_sensitive(arrow_button, FALSE);

	gtk_widget_show_all(box);

	gtk_container_add(GTK_CONTAINER(button), box);

	button->priv->button = real_button;
	button->priv->arrow = arrow;
	button->priv->arrow_button = arrow_button;
	button->priv->box = box;

	g_signal_connect(arrow_button, "toggled",
					G_CALLBACK(arrow_button_toggled_cb), button);
	g_signal_connect(arrow_button, "button-press-event",
					G_CALLBACK(arrow_button_button_press_event_cb), button);
}

static void gtk_menu_tool_toggle_button_destroy(GtkObject* object)
{
	GtkMenuToolToggleButton* button;

	button = GTK_MENU_TOOL_TOGGLE_BUTTON(object);

	if (button->priv->menu)
	{
		g_signal_handlers_disconnect_by_func(button->priv->menu,
											(gpointer) G_CALLBACK(menu_deactivate_cb),
											button);
		gtk_menu_detach(button->priv->menu);

		g_signal_handlers_disconnect_by_func(button->priv->arrow_button,
											(gpointer) G_CALLBACK(arrow_button_toggled_cb),
											button);
		g_signal_handlers_disconnect_by_func(button->priv->arrow_button,
											(gpointer) G_CALLBACK(arrow_button_button_press_event_cb),
											button);
	}

	GTK_OBJECT_CLASS(gtk_menu_tool_toggle_button_parent_class)->destroy(object);
}

/**
 * gtk_menu_tool_button_new:
 * @icon_widget: a widget that will be used as icon widget, or %NULL
 * @label: a string that will be used as label, or %NULL
 *
 * Creates a new #GtkMenuToolButton using @icon_widget as icon and
 * @label as label.
 *
 * Return value: the new #GtkMenuToolButton
 *
 * Since: 2.6
 **/
GtkToolItem*
gtk_menu_tool_toggle_button_new(GtkWidget* icon_widget, const gchar* label)
{
	void* button;

	button = g_object_new(GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, NULL);

	if (label)
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(button), label);

	if (icon_widget)
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(button), icon_widget);

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
GtkToolItem*
gtk_menu_tool_toggle_button_new_from_stock(const gchar* stock_id)
{
	void* button;

	g_return_val_if_fail(stock_id != NULL, NULL);

	button = g_object_new(GTK_TYPE_MENU_TOOL_TOGGLE_BUTTON, "stock-id", stock_id,
						NULL);

	return GTK_TOOL_ITEM(button);
}

/* Callback for the "deactivate" signal on the pop-up menu.
 * This is used so that we unset the state of the toggle button
 * when the pop-up menu disappears.
 */
static int menu_deactivate_cb(GtkMenuShell* menu_shell,
							  GtkMenuToolToggleButton* button)
{
	GtkMenuToolToggleButtonPrivate* priv = button->priv;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->arrow_button), FALSE);

	return TRUE;
}

static void menu_detacher(GtkWidget* widget, GtkMenu* menu)
{
	GtkMenuToolToggleButtonPrivate* priv = GTK_MENU_TOOL_TOGGLE_BUTTON(
																	widget)->priv;

	g_return_if_fail(priv->menu == menu);

	priv->menu = NULL;
}

/**
 * gtk_menu_tool_button_set_menu:
 * @button: a #GtkMenuToolButton
 * @menu: the #GtkMenu associated with #GtkMenuToolButton
 *
 * Sets the #GtkMenu that is popped up when the user clicks on the arrow.
 * If @menu is NULL, the arrow button becomes insensitive.
 *
 * Since: 2.6
 **/
void gtk_menu_tool_toggle_button_set_menu(GtkMenuToolToggleButton* button,
										  GtkWidget* menu)
{
	GtkMenuToolToggleButtonPrivate* priv;

	g_return_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button));
	g_return_if_fail(GTK_IS_MENU(menu) || menu == NULL);

	priv = button->priv;

	if (priv->menu != GTK_MENU(menu))
	{
		if (priv->menu && GTK_WIDGET_VISIBLE(priv->menu))
			gtk_menu_shell_deactivate(GTK_MENU_SHELL(priv->menu));

		if (priv->menu)
		{
			g_signal_handlers_disconnect_by_func(priv->menu,
												(gpointer) G_CALLBACK(menu_deactivate_cb),
												button);
			gtk_menu_detach(priv->menu);
		}

		priv->menu = GTK_MENU(menu);

		if (priv->menu)
		{
			gtk_menu_attach_to_widget(priv->menu, GTK_WIDGET(button), menu_detacher);

			gtk_widget_set_sensitive(priv->arrow_button, TRUE);

			g_signal_connect(priv->menu, "deactivate",
							G_CALLBACK(menu_deactivate_cb), button);
		}
		else
			gtk_widget_set_sensitive(priv->arrow_button, FALSE);
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
GtkWidget*
gtk_menu_tool_toggle_button_get_menu(GtkMenuToolToggleButton* button)
{
	g_return_val_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button), NULL);

	return GTK_WIDGET(button->priv->menu);
}

/**
 * gtk_menu_tool_button_set_arrow_tooltip:
 * @button: a #GtkMenuToolButton
 * @tooltips: the #GtkTooltips object to be used
 * @tip_text: text to be used as tooltip text for tool_item
 * @tip_private: text to be used as private tooltip text
 *
 * Sets the #GtkTooltips object to be used for arrow button which
 * pops up the menu. See gtk_tool_item_set_tooltip() for setting
 * a tooltip on the whole #GtkMenuToolButton.
 *
 * Since: 2.6
 *
 * Deprecated: 2.12: Use gtk_menu_tool_button_set_arrow_tooltip_text()
 * instead.
 **/
void gtk_menu_tool_toggle_button_set_arrow_tooltip(GtkMenuToolToggleButton*
												   button, GtkTooltips* tooltips,
												   const gchar* tip_text, const gchar* tip_private)
{
	g_return_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button));

	gtk_tooltips_set_tip(tooltips, button->priv->arrow_button, tip_text,
						tip_private);
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
void gtk_menu_tool_toggle_button_set_arrow_tooltip_text(
														GtkMenuToolToggleButton* button, const gchar* text)
{
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
void gtk_menu_tool_toggle_button_set_arrow_tooltip_markup(
														  GtkMenuToolToggleButton* button, const gchar* markup)
{
	g_return_if_fail(GTK_IS_MENU_TOOL_TOGGLE_BUTTON(button));

	gtk_widget_set_tooltip_markup(button->priv->arrow_button, markup);
}

//#define __GTK_MENU_TOOL_BUTTON_C__
//#include "gtkaliasdef.c"
