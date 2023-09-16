#include "AbstractToolItem.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_object_...
#include <glib.h>         // for g_error, g_warning, gchar

#include "gui/toolbarMenubar/AbstractItem.h"  // for AbstractItem

class ActionHandler;

AbstractToolItem::AbstractToolItem(std::string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem):
        AbstractItem(std::move(id), handler, type, menuitem) {}

AbstractToolItem::AbstractToolItem(std::string id): AbstractItem(id) {}

AbstractToolItem::~AbstractToolItem() = default;

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

void AbstractToolItem::toolButtonCallback(GtkToolButton* toolbutton, AbstractToolItem* item) {
    if (toolbutton && GTK_IS_TOGGLE_TOOL_BUTTON(toolbutton)) {
        bool selected = gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton));

        // ignore this event... GTK Broadcast to much events, e.g. if you call set_active
        if (item->toolToggleButtonActive == selected) {
            return;
        }

        // don't allow deselect this button
        if (item->toolToggleOnlyEnable && !selected) {
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(toolbutton), true);
            return;
        }

        item->toolToggleButtonActive = selected;
    }

    item->activated(nullptr, toolbutton);
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
