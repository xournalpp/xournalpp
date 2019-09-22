#include "MenuItem.h"

MenuItem::MenuItem(ActionHandler* handler, GtkWidget* widget, ActionType type, ActionGroup group)
 : AbstractItem("", handler, type, widget)
{
	this->group = group;
}

MenuItem::~MenuItem()
{
}
