#include "MenuItem.h"

MenuItem::MenuItem(ActionHandler * handler, GtkWidget * widget, ActionType type) :
	AbstractItem(NULL, handler, type, widget) {
}

MenuItem::MenuItem(ActionHandler * handler, GtkWidget * widget, ActionType type, ActionGroup group) :
	AbstractItem(NULL, handler, type, widget) {
	this->group = group;
}

MenuItem::~MenuItem() {
}
