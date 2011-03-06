#include "AbstractItem.h"

AbstractItem::AbstractItem(String id, ActionHandler * handler, ActionType action, GtkWidget * menuitem) {
	this->id = id;
	this->handler = handler;
	this->action = action;
	this->menuitem = NULL;
	this->menuSignalHandler = 0;
	this->group = GROUP_NOGROUP;
	this->enabled = true;

	ActionEnabledListener::registerListener(handler);
	ActionSelectionListener::registerListener(handler);

	if (menuitem) {
		menuSignalHandler = g_signal_connect(menuitem, "activate", G_CALLBACK(&menuCallback), this);
		gtk_object_ref(GTK_OBJECT(menuitem));
		this->menuitem = menuitem;
	}
}

AbstractItem::~AbstractItem() {
	if (this->menuitem) {
		g_signal_handler_disconnect(this->menuitem, menuSignalHandler);
		gtk_object_unref(GTK_OBJECT(this->menuitem));
	}
}

void AbstractItem::menuCallback(GtkMenuItem * menuitem, AbstractItem * toolItem) {
	toolItem->activated(NULL, menuitem, NULL);
}

void AbstractItem::actionSelected(ActionGroup group, ActionType action) {
	if (this->group == group) {

		if (this->menuitem && GTK_IS_CHECK_MENU_ITEM(this->menuitem)) {
			if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(this->menuitem)) != (this->action == action)) {
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->menuitem), this->action == action);
			}
		}
		selected(group, action);
	}
}

/**
 * Override this method
 */
void AbstractItem::selected(ActionGroup group, ActionType action) {
}

void AbstractItem::actionEnabledAction(ActionType action, bool enabled) {
	if (this->action == action) {
		this->enabled = enabled;
		enable(enabled);
		if (this->menuitem) {
			gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), enabled);
		}
	}
}

void AbstractItem::activated(GdkEvent * event, GtkMenuItem * menuitem, GtkToolButton * toolbutton) {
	bool selected = true;

	if (menuitem && GTK_IS_CHECK_MENU_ITEM(menuitem)) {
		selected = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));
	} else if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton)) {
		selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));
	}

	handler->actionPerformed(action, group, event, menuitem, toolbutton, selected);
}

String AbstractItem::getId() {
	return id;
}

void AbstractItem::setTmpDisabled(bool disabled) {
	bool ena = false;
	if (disabled) {
		ena = false;
	} else {
		ena = this->enabled;
	}

	enable(ena);
	if (this->menuitem) {
		gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), ena);
	}
}

void AbstractItem::enable(bool enabled) {
}

