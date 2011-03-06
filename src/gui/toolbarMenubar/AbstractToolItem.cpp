#include "AbstractToolItem.h"

AbstractToolItem::AbstractToolItem(String id, ActionHandler * handler, ActionType type, GtkWidget * menuitem) :
	AbstractItem(id, handler, type, menuitem) {
	this->item = NULL;
	this->popupMenu = NULL;
	this->used = false;
}

AbstractToolItem::~AbstractToolItem() {
	if (item) {
		gtk_object_unref(GTK_OBJECT(item));
	}
	if (this->popupMenu) {
		gtk_object_unref(GTK_OBJECT(this->popupMenu));
	}
}

void AbstractToolItem::selected(ActionGroup group, ActionType action) {
	if (item) {
		if (!GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
			g_warning("selected action %i which is not a toggle action!", action);
			return;
		}
		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(item)) != (this->action == action)) {
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), (this->action == action));
		}
	}
}

void AbstractToolItem::toolButtonCallback(GtkToolButton * toolbutton, AbstractToolItem * item) {
	item->activated(NULL, NULL, toolbutton);
}

GtkToolItem * AbstractToolItem::createItem(bool horizontal) {
	if (item) {
		return item;
	}

	item = newItem();
	g_object_ref(item);

	if (GTK_IS_TOOL_ITEM(item)) {
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
	}
	if (GTK_IS_TOOL_BUTTON(item) || GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
		g_signal_connect(item, "clicked", G_CALLBACK(&toolButtonCallback), this);
	}
	gtk_object_ref(GTK_OBJECT(item));
	return item;

}

void AbstractToolItem::setPopupMenu(GtkWidget * popupMenu) {
	if (this->popupMenu) {
		g_object_unref(this->popupMenu);
	}
	if (popupMenu) {
		g_object_ref(popupMenu);
	}

	this->popupMenu = popupMenu;
}

GtkWidget * AbstractToolItem::getPopupMenu() {
	return this->popupMenu;
}

bool AbstractToolItem::isUsed() {
	return used;
}

void AbstractToolItem::setUsed(bool used) {
	this->used = used;
}

void AbstractToolItem::enable(bool enabled) {
	if (this->item) {
		gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
	}
}
