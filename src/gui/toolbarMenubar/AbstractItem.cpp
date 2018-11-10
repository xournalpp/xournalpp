#include "AbstractItem.h"

AbstractItem::AbstractItem(string id, ActionHandler* handler, ActionType action, GtkWidget* menuitem)
{
	XOJ_INIT_TYPE(AbstractItem);
	this->id = id;
	this->handler = handler;
	this->action = action;
	this->menuitem = NULL;
	this->menuSignalHandler = 0;
	this->group = GROUP_NOGROUP;
	this->enabled = true;

	ActionEnabledListener::registerListener(handler);
	ActionSelectionListener::registerListener(handler);

	if (menuitem)
	{
		// Other signal available: "toggled", currently not sure, if this may fix some bugs or generate other...
		menuSignalHandler = g_signal_connect(menuitem, "activate", G_CALLBACK(&menuCallback), this);
		g_object_ref(G_OBJECT(menuitem));
		this->menuitem = menuitem;
	}
}

AbstractItem::~AbstractItem()
{
	XOJ_CHECK_TYPE(AbstractItem);

	if (this->menuitem)
	{
		g_signal_handler_disconnect(this->menuitem, menuSignalHandler);
		g_object_unref(G_OBJECT(this->menuitem));
	}

	XOJ_RELEASE_TYPE(AbstractItem);
}

void AbstractItem::menuCallback(GtkMenuItem* menuitem, AbstractItem* toolItem)
{
	XOJ_CHECK_TYPE_OBJ(toolItem, AbstractItem);
	toolItem->activated(NULL, menuitem, NULL);
}

void AbstractItem::actionSelected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(AbstractItem);

	if (this->group != group)
	{
		return;
	}

	if (this->menuitem && GTK_IS_CHECK_MENU_ITEM(this->menuitem))
	{
		bool selected = this->action == action;
		if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(this->menuitem)) != selected)
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->menuitem), selected);
		}
	}
	selected(group, action);
}

/**
 * Override this method
 */
void AbstractItem::selected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(AbstractItem);
}

void AbstractItem::actionEnabledAction(ActionType action, bool enabled)
{
	XOJ_CHECK_TYPE(AbstractItem);

	if (this->action != action)
	{
		return;
	}

	this->enabled = enabled;
	enable(enabled);
	if (this->menuitem)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), enabled);
	}
}

void AbstractItem::activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton)
{
	XOJ_CHECK_TYPE(AbstractItem);

	bool selected = true;

	if (menuitem)
	{
		if (GTK_IS_CHECK_MENU_ITEM(menuitem))
		{
			selected = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
			if (!selected && gtk_check_menu_item_get_draw_as_radio(GTK_CHECK_MENU_ITEM(menuitem)))
			{
				// Unselect radio menu item
				return;
			}
		}
	}
	else if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton))
	{
		selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));
	}


	handler->actionPerformed(action, group, event, menuitem, toolbutton, selected);
}

string AbstractItem::getId()
{
	XOJ_CHECK_TYPE(AbstractItem);

	return id;
}

void AbstractItem::setTmpDisabled(bool disabled)
{
	XOJ_CHECK_TYPE(AbstractItem);

	bool ena = false;
	if (disabled)
	{
		ena = false;
	}
	else
	{
		ena = this->enabled;
	}

	enable(ena);
	if (this->menuitem)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), ena);
	}
}

void AbstractItem::enable(bool enabled)
{
	XOJ_CHECK_TYPE(AbstractItem);
}

bool AbstractItem::isEnabled()
{
	XOJ_CHECK_TYPE(AbstractItem);

	return this->enabled;
}
