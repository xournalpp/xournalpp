#include "AbstractItem.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_object_unref, g_signal_hand...

AbstractItem::AbstractItem(std::string id, ActionHandler* handler, ActionType action, GtkWidget* menuitem):
        action(action), id(std::move(id)), handler(handler) {
    ActionEnabledListener::registerListener(handler);
    ActionSelectionListener::registerListener(handler);

    if (menuitem) {
        setMenuItem(menuitem);
    }
}

AbstractItem::~AbstractItem() {
    if (this->menuitem) {
        g_signal_handler_disconnect(this->menuitem, menuSignalHandler);
        g_object_unref(G_OBJECT(this->menuitem));
    }
}

/**
 * Register a menu item. If there is already one registered, the new one will be ignored
 */
void AbstractItem::setMenuItem(GtkWidget* menuitem) {
    if (this->menuitem != nullptr) {
        g_warning("The menu item %i / %s has already a menu item registered!", action,
                  ActionType_toString(action).c_str());
        return;
    }

    menuSignalHandler = g_signal_connect(
            menuitem, "activate",
            G_CALLBACK(+[](GtkMenuItem* menuitem, AbstractItem* self) { self->activated(menuitem, nullptr); }), this);

    g_object_ref(G_OBJECT(menuitem));
    this->menuitem = menuitem;

    if (GTK_IS_CHECK_MENU_ITEM(menuitem)) {
        checkMenuItem = !gtk_check_menu_item_get_draw_as_radio(GTK_CHECK_MENU_ITEM(menuitem));
    }
}

void AbstractItem::actionSelected(ActionGroup group, ActionType action) {
    if (this->group != group) {
        return;
    }

    itemActive = this->action == action;

    if (this->menuitem && GTK_IS_CHECK_MENU_ITEM(this->menuitem)) {
        if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(this->menuitem)) != itemActive) {
            if (checkMenuItem) {
                ignoreNextCheckMenuEvent = true;
            }
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->menuitem), itemActive);
        }
    }
    selected(group, action);
}

/**
 * Override this method
 */
void AbstractItem::selected(ActionGroup group, ActionType action) {}

void AbstractItem::actionEnabledAction(ActionType action, bool enabled) {
    if (this->action != action) {
        return;
    }

    this->enabled = enabled;
    enable(enabled);
    if (this->menuitem) {
        gtk_widget_set_sensitive(GTK_WIDGET(this->menuitem), enabled);
    }
}

void AbstractItem::activated(GtkMenuItem* menuitem, GtkToolButton* toolbutton) {
    bool selected = true;

    if (menuitem) {
        if (GTK_IS_CHECK_MENU_ITEM(menuitem)) {
            selected = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem));

            if (gtk_check_menu_item_get_draw_as_radio(GTK_CHECK_MENU_ITEM(menuitem))) {
                if (itemActive && !selected) {
                    // Unselect radio menu item, select again
                    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(this->menuitem), true);
                    return;
                }

                if (itemActive == selected) {
                    // State not changed, this event is probably from GTK generated
                    return;
                }

                if (!selected) {
                    // Unselect radio menu item
                    return;
                }
            }
        }
    } else if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton)) {
        selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));
    }

    if (checkMenuItem && ignoreNextCheckMenuEvent) {
        ignoreNextCheckMenuEvent = false;
        return;
    }

    actionPerformed(action, group, toolbutton, selected);
}

void AbstractItem::actionPerformed(ActionType action, ActionGroup group, GtkToolButton* toolbutton, bool selected) {
    handler->actionPerformed(action, group, toolbutton, selected);
}

auto AbstractItem::getId() const -> std::string { return id; }

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

void AbstractItem::enable(bool enabled) {}

auto AbstractItem::isEnabled() const -> bool { return this->enabled; }

auto AbstractItem::getActionType() -> ActionType { return this->action; }
