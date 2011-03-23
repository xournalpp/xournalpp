#include "ToolButton.h"
#include "../widgets/gtkmenutooltogglebutton.h"
// TODO: AA: type check

ToolButton::ToolButton(ActionHandler * handler, String id, ActionType type, String stock, String description, GtkWidget * menuitem) :
	AbstractToolItem(id, handler, type, menuitem) {
	this->stock = stock;
	this->gui = NULL;
	this->description = description;
}

ToolButton::ToolButton(ActionHandler * handler, GladeGui * gui, String id, ActionType type, String iconName, String description, GtkWidget * menuitem) :
	AbstractToolItem(id, handler, type, menuitem) {
	this->iconName = iconName;
	this->gui = gui;
	this->description = description;
}

ToolButton::ToolButton(ActionHandler * handler, GladeGui * gui, String id, ActionType type, ActionGroup group, String iconName, String description,
		GtkWidget * menuitem) :
	AbstractToolItem(id, handler, type, menuitem) {
	this->iconName = iconName;
	this->gui = gui;
	this->description = description;
	this->group = group;
}

ToolButton::~ToolButton() {
}

void ToolButton::updateDescription(String description) {
	this->description = description;
	if (GTK_IS_TOOL_ITEM(item)) {
		gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());
		gtk_tool_button_set_label(GTK_TOOL_BUTTON(item), description.c_str());
	}
}

GtkToolItem * ToolButton::newItem() {
	GtkToolItem * it;

	if (!stock.isEmpty()) {
		if (popupMenu) {
			it = gtk_menu_tool_button_new_from_stock(stock.c_str());
			gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it), popupMenu);
		} else {
			it = gtk_tool_button_new_from_stock(stock.c_str());
		}
	} else if (group != GROUP_NOGROUP) {
		if (popupMenu) {
			it = gtk_menu_tool_toggle_button_new(gui->loadIcon(iconName.c_str()), description.c_str());
			gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
		} else {
			it = gtk_toggle_tool_button_new();
			gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), gui->loadIcon(iconName.c_str()));
		}
	} else {
		if (popupMenu) {
			it = gtk_menu_tool_button_new(gui->loadIcon(iconName.c_str()), description.c_str());
			gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(it), popupMenu);
		} else {
			it = gtk_tool_button_new(gui->loadIcon(iconName.c_str()), description.c_str());
		}
	}
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(it), description.c_str());
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), description.c_str());

	return it;
}
