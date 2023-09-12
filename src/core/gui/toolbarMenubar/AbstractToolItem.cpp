#include "AbstractToolItem.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_object_...
#include <glib.h>         // for g_error, g_warning, gchar

class ActionHandler;

AbstractToolItem::AbstractToolItem(std::string id, ActionHandler* handler, ActionType type):
        action(action), id(std::move(id)), handler(handler) {
    ActionEnabledListener::registerListener(handler);
    ActionSelectionListener::registerListener(handler);
}

AbstractToolItem::AbstractToolItem(std::string id): id(std::move(id)) {}

AbstractToolItem::~AbstractToolItem() = default;

////////////////////////////////:

void AbstractToolItem::actionSelected(ActionGroup group, ActionType action) {
    if (this->group != group) {
        return;
    }
    selected(group, action);
}

void AbstractToolItem::actionEnabledAction(ActionType action, bool enabled) {
    if (this->action != action) {
        return;
    }

    this->enabled = enabled;
    enable(enabled);
}

auto AbstractToolItem::getId() const -> const std::string& { return id; }

void AbstractToolItem::setTmpDisabled(bool disabled) {
    bool ena = false;
    if (disabled) {
        ena = false;
    } else {
        ena = this->enabled;
    }

    enable(ena);
}

void AbstractToolItem::enable(bool enabled) {}

auto AbstractToolItem::isEnabled() const -> bool { return this->enabled; }

auto AbstractToolItem::getActionType() -> ActionType { return this->action; }

/////////////////////////////////////

auto AbstractToolItem::getItem() const -> GtkWidget* { return this->item.get(); }

void AbstractToolItem::selected(ActionGroup group, ActionType action) {
    if (!this->item) {
        return;
    }

    if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item.get())) {
        g_warning("selected action %i (group=%i) which is not a toggle action!", action, group);
        return;
    }

    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item.get())) != (this->action == action)) {
        this->toolToggleButtonActive = (this->action == action);
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item.get()), this->toolToggleButtonActive);
    }
}

GtkToolItem* AbstractToolItem::createToolItem(bool horizontal) {
    GtkWidget* item = createItem(horizontal);
    gtk_widget_set_can_focus(item, false);  // todo(gtk4) not necessary anymore
    if (GTK_IS_TOOL_ITEM(item)) {
        return GTK_TOOL_ITEM(item);
    }

    // Wrap in a GtkToolItem
    GtkToolItem* wrap = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(wrap), item);
    this->item.reset(GTK_WIDGET(wrap), xoj::util::adopt);
    return wrap;
}

auto AbstractToolItem::isUsed() const -> bool { return used; }

void AbstractToolItem::setUsed(bool used) { this->used = used; }
