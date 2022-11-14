#include "AbstractToolItem.h"

#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_object_...
#include <glib.h>         // for g_error, g_warning, gchar

#include "gui/toolbarMenubar/AbstractItem.h"  // for AbstractItem

class ActionHandler;

AbstractToolItem::AbstractToolItem(std::string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem):
        AbstractItem(std::move(id), handler, type, menuitem) {}

AbstractToolItem::~AbstractToolItem() {
    this->item = nullptr;
    if (this->popupMenu) {
        g_object_unref(G_OBJECT(this->popupMenu));
        this->popupMenu = nullptr;
    }
}

auto AbstractToolItem::getItem() const -> GtkToolItem* {
    return this->item;
}

void AbstractToolItem::selected(ActionGroup group, ActionType action) {
    if (this->item == nullptr) {
        return;
    }

    if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
        g_warning("selected action %i (group=%i) which is not a toggle action!", action, group);
        return;
    }

    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action)) {
        this->toolToggleButtonActive = (this->action == action);
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
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

auto AbstractToolItem::createItem(bool horizontal) -> GtkToolItem* {
    if (this->item) {
        return this->item;
    }

    this->item = createTmpItem(horizontal);
    g_object_ref(this->item);

    if (GTK_IS_TOOL_BUTTON(this->item) || GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
        g_signal_connect(this->item, "clicked", G_CALLBACK(&toolButtonCallback), this);
    }

    return this->item;
}

auto AbstractToolItem::createTmpItem(bool horizontal) -> GtkToolItem* {
    GtkToolItem* item = newItem();

    if (GTK_IS_TOOL_ITEM(item)) {
        gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
    }

    gtk_widget_show_all(GTK_WIDGET(item));
    return item;
}

void AbstractToolItem::setPopupMenu(GtkWidget* popupMenu) {
    if (this->popupMenu) {
        g_object_unref(this->popupMenu);
    }
    if (popupMenu) {
        g_object_ref(popupMenu);
    }

    this->popupMenu = popupMenu;
}

auto AbstractToolItem::isUsed() const -> bool { return used; }

void AbstractToolItem::setUsed(bool used) { this->used = used; }

/**
 * Enable / Disable the tool item
 */
void AbstractToolItem::enable(bool enabled) {
    if (this->item) {
        gtk_widget_set_sensitive(GTK_WIDGET(this->item), enabled);
    }
}

auto AbstractToolItem::getPixbufFromImageIconName() const -> GdkPixbuf* {
    GtkImage* image = GTK_IMAGE(getNewToolIcon());
    if (gtk_image_get_storage_type(image) != GTK_IMAGE_ICON_NAME) {
        g_error("getPixbufFromImageIconName is only intended for image type: GTK_IMAGE_ICON_NAME");
    }
    const gchar* iconName = nullptr;
    gtk_image_get_icon_name(image, &iconName, nullptr);
    return gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), iconName, 16, static_cast<GtkIconLookupFlags>(0),
                                    nullptr);
}
