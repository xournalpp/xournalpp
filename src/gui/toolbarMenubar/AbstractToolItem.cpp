#include "AbstractToolItem.h"
#include "gui/toolbarMenubar/ToolbarUtil.h"


AbstractToolItem::AbstractToolItem(string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem)
 : AbstractItem(id, handler, type, menuitem)
{
	XOJ_INIT_TYPE(AbstractToolItem);

	this->item = NULL;
	this->popupMenu = NULL;
	this->used = false;

	this->toolToggleButtonActive = false;
	this->toolToggleOnlyEnable = false;
}

AbstractToolItem::~AbstractToolItem()
{
	XOJ_CHECK_TYPE(AbstractToolItem);

	this->item = NULL;
	if (this->popupMenu)
	{
		g_object_unref(G_OBJECT(this->popupMenu));
		this->popupMenu = NULL;
	}

	XOJ_RELEASE_TYPE(AbstractToolItem);
}

void AbstractToolItem::selected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(AbstractToolItem);

	if (this->item == NULL)
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
	XOJ_CHECK_TYPE_OBJ(item, AbstractToolItem);

	if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton))
	{
		bool selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));

		// ignor this event... GTK Broadcast to much eventes, e.g. if you call set_active
		if (item->toolToggleButtonActive == selected)
		{
			return;
		}

		// don't allow disselect this button
		if (item->toolToggleOnlyEnable && selected == false)
		{
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton), true);
			return;
		}

		item->toolToggleButtonActive = selected;
	}

	item->activated(NULL, NULL, toolbutton);
}

GtkToolItem* AbstractToolItem::createItem(bool horizontal)
{
	XOJ_CHECK_TYPE(AbstractToolItem);

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
	XOJ_CHECK_TYPE(AbstractToolItem);

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
	XOJ_CHECK_TYPE(AbstractToolItem);

	return used;
}

void AbstractToolItem::setUsed(bool used)
{
	XOJ_CHECK_TYPE(AbstractToolItem);

	this->used = used;
}

void AbstractToolItem::enable(bool enabled)
{
	XOJ_CHECK_TYPE(AbstractToolItem);

	if (this->item)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
	}
}
