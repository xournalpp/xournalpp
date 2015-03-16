#include "ToolButton.h"
#include "../widgets/gtkmenutooltogglebutton.h"

ToolButton::ToolButton(ActionHandler* handler, String id, ActionType type,
                       String stock, String description, GtkWidget* menuitem) :
	AbstractToolItem(id, handler, type, menuitem)
{
	XOJ_INIT_TYPE(ToolButton);

	this->stock = stock;
	this->gui = NULL;
	this->description = description;
}

ToolButton::ToolButton(ActionHandler* handler, GladeGui* gui, String id,
                       ActionType type, String iconName, String description, GtkWidget* menuitem) :
	AbstractToolItem(id, handler, type, menuitem)
{
	XOJ_INIT_TYPE(ToolButton);

	this->iconName = iconName;
	this->gui = gui;
	this->description = description;
}

ToolButton::ToolButton(ActionHandler* handler, GladeGui* gui, String id,
                       ActionType type, ActionGroup group, bool toolToggleOnlyEnable, String iconName,
                       String description, GtkWidget* menuitem) :
	AbstractToolItem(id, handler, type, menuitem)
{
	XOJ_INIT_TYPE(ToolButton);

	this->iconName = iconName;
	this->gui = gui;
	this->description = description;
	this->group = group;
	this->toolToggleOnlyEnable = toolToggleOnlyEnable;
}

ToolButton::~ToolButton()
{
	XOJ_RELEASE_TYPE(ToolButton);
}

void ToolButton::updateDescription(String description)
{
	XOJ_CHECK_TYPE(ToolButton);

	this->description = description;
	if (GTK_IS_TOOL_ITEM(item))
	{
		gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), CSTR(description));
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), CSTR(description));
	}
}

GtkToolItem* ToolButton::newItem()
{
	XOJ_CHECK_TYPE(ToolButton);

	GtkToolItem* it;

	if (!stock.isEmpty())
	{
		if (popupMenu)
		{
			it = gtk_menu_tool_button_new_from_stock(CSTR(stock));
			gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it), popupMenu);
		}
		else
		{
			it = gtk_tool_button_new_from_stock(CSTR(stock));
		}
	}
	else if (group != GROUP_NOGROUP)
	{
		if (popupMenu)
		{
			it = gtk_menu_tool_toggle_button_new(this->gui->loadIcon(CSTR(iconName)),
			                                     CSTR(description));
			gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it),
			                                     popupMenu);
		}
		else
		{
			it = gtk_toggle_tool_button_new();
			gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it),
			                                this->gui->loadIcon(CSTR(iconName)));
		}
	}
	else
	{
		if (popupMenu)
		{
			it = gtk_menu_tool_button_new(this->gui->loadIcon(CSTR(iconName)),
			                              CSTR(description));
			gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it), popupMenu);
		}
		else
		{
			it = gtk_tool_button_new(this->gui->loadIcon(CSTR(iconName)),
			                         CSTR(description));
		}
	}
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(it), CSTR(description));
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), CSTR(description));

	return it;
}

String ToolButton::getToolDisplayName()
{
	XOJ_CHECK_TYPE(ToolButton);

	return this->description;
}

GtkWidget* ToolButton::getNewToolIconImpl()
{
	XOJ_CHECK_TYPE(ToolButton);

	if (!stock.isEmpty())
	{
		return gtk_image_new_from_stock(CSTR(stock), GTK_ICON_SIZE_SMALL_TOOLBAR);
	}
	else if (this->gui != NULL)
	{
		return this->gui->loadIcon(CSTR(iconName));
	}
	return NULL;
}
