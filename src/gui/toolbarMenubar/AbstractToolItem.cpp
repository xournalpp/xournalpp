#include "AbstractToolItem.h"

AbstractToolItem::AbstractToolItem(string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem)
 : AbstractItem(id, handler, type, menuitem)
{
}

AbstractToolItem::~AbstractToolItem()
{
	this->item = nullptr;
	if (this->popupMenu)
	{
		g_object_unref(G_OBJECT(this->popupMenu));
		this->popupMenu = nullptr;
	}
}

void AbstractToolItem::selected(ActionGroup group, ActionType action)
{
	if (this->item == nullptr)
	{
		return;
	}

	if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
	{
		g_warning("selected action %i (group=%i) which is not a toggle action!", action, group);
		return;
	}

	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action))
	{
		this->toolToggleButtonActive = (this->action == action);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
	}
}

void AbstractToolItem::toolButtonCallback(GtkToolButton* toolbutton, AbstractToolItem* item)
{
	if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton))
	{
		bool selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));

		// ignore this event... GTK Broadcast to much events, e.g. if you call set_active
		if (item->toolToggleButtonActive == selected)
		{
			return;
		}

		// don't allow deselect this button
		if (item->toolToggleOnlyEnable && selected == false)
		{
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton), true);
			return;
		}

		item->toolToggleButtonActive = selected;
	}

	item->activated(nullptr, nullptr, toolbutton);
}

GtkToolItem* AbstractToolItem::createItem(bool horizontal)
{
	if (this->item)
	{
		return this->item;
	}

	this->item = createTmpItem(horizontal);
	g_object_ref(this->item);

	if (GTK_IS_TOOL_BUTTON(this->item) || GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
	{
		g_signal_connect(this->item, "clicked", G_CALLBACK(&toolButtonCallback), this);
	}

	return this->item;
}

GtkToolItem* AbstractToolItem::createTmpItem(bool horizontal)
{
	GtkToolItem* item = newItem();

	if (GTK_IS_TOOL_ITEM(item))
	{
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
	}

	gtk_widget_show_all(GTK_WIDGET(item));
	return item;
}

void AbstractToolItem::setPopupMenu(GtkWidget* popupMenu)
{
	if (this->popupMenu)
	{
		g_object_unref(this->popupMenu);
	}
	if (popupMenu)
	{
		g_object_ref(popupMenu);
	}

	this->popupMenu = popupMenu;
}

bool AbstractToolItem::isUsed()
{
	return used;
}

void AbstractToolItem::setUsed(bool used)
{
	this->used = used;
}

/**
 * Enable / Disable the tool item
 */
void AbstractToolItem::enable(bool enabled)
{
	if (this->item)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
	}
}
