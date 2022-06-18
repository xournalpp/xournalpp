#include "MenuItem.h"

#include <string>  // for allocator, string

#include "gui/toolbarMenubar/AbstractItem.h"  // for AbstractItem

class ActionHandler;

MenuItem::MenuItem(ActionHandler* handler, GtkWidget* widget, ActionType type, ActionGroup group):
        AbstractItem("", handler, type, widget) {
    this->group = group;
}

MenuItem::~MenuItem() = default;
