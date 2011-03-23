#include "AbstractToolItem.h"

AbstractToolItem::AbstractToolItem(String id, ActionHandler * handler, ActionType type, GtkWidget * menuitem) :
	AbstractItem(id, handler, type, menuitem) {
	XOJ_INIT_TYPE(AbstractToolItem);

	this->item = NULL;
	this->popupMenu = NULL;
	this->used = false;
}

AbstractToolItem::~AbstractToolItem() {
	XOJ_CHECK_TYPE(XmlNode);

	if (this->item) {
		gtk_object_unref(GTK_OBJECT(this->item));
	}
	if (this->popupMenu) {
		gtk_object_unref(GTK_OBJECT(this->popupMenu));
	}

	XOJ_RELEASE_TYPE(AbstractToolItem);
}

void AbstractToolItem::selected(ActionGroup group, ActionType action) {
	XOJ_CHECK_TYPE(AbstractToolItem);

	if (this->item) {
		if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
			g_warning("selected action %i which is not a toggle action!", action);
			return;
		}
		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action)) {
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), (this->action == action));
		}
	}
}

void AbstractToolItem::toolButtonCallback(GtkToolButton * toolbutton, AbstractToolItem * item) {
	item->activated(NULL, NULL, toolbutton);
}

GtkToolItem * AbstractToolItem::createItem(bool horizontal) {
	XOJ_CHECK_TYPE_RET(AbstractToolItem, NULL);

	if (this->item) {
		return this->item;
	}

	this->item = newItem();
	g_object_ref(this->item);

	if (GTK_IS_TOOL_ITEM(this->item)) {
		gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(this->item), false);
	}
	if (GTK_IS_TOOL_BUTTON(this->item) || GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
		g_signal_connect(this->item, "clicked", G_CALLBACK(&toolButtonCallback), this);
	}
	gtk_object_ref(GTK_OBJECT(this->item));
	return this->item;
}

void AbstractToolItem::setPopupMenu(GtkWidget * popupMenu) {
	XOJ_CHECK_TYPE(AbstractToolItem);

	if (this->popupMenu) {
		g_object_unref(this->popupMenu);
	}
	if (popupMenu) {
		g_object_ref(popupMenu);
	}

	this->popupMenu = popupMenu;
}

GtkWidget * AbstractToolItem::getPopupMenu() {
	XOJ_CHECK_TYPE_RET(AbstractToolItem, NULL);

	return this->popupMenu;
}

bool AbstractToolItem::isUsed() {
	XOJ_CHECK_TYPE_RET(AbstractToolItem, false);

	return used;
}

void AbstractToolItem::setUsed(bool used) {
	XOJ_CHECK_TYPE(AbstractToolItem);

	this->used = used;
}

void AbstractToolItem::enable(bool enabled) {
	XOJ_CHECK_TYPE(AbstractToolItem);

	if (this->item) {
		gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
	}
}
