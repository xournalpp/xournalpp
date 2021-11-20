#include "ToolButton.h"

#include <utility>

#include "Gtk4Util.h"

using std::string;

// Todo (gtk4): I think this whole concept must be replaced by a sort of GMenu - generator

ToolButton::ToolButton(ActionHandler* handler, string id, ActionType type, string iconName, string description,
                       GtkWidget* menuitem):
        AbstractToolItem(std::move(id), handler, type, menuitem) {
    this->iconName = std::move(iconName);
    this->description = std::move(description);
}

ToolButton::ToolButton(ActionHandler* handler, string id, ActionType type, ActionGroup group, bool toolToggleOnlyEnable,
                       string iconName, string description, GtkWidget* menuitem):
        AbstractToolItem(std::move(id), handler, type, menuitem) {
    this->iconName = std::move(iconName);
    this->description = std::move(description);
    this->group = group;
    this->toolToggleOnlyEnable = toolToggleOnlyEnable;
}

/**
 * Register a popup menu entry, create a popup menu, if none is there
 *
 * @param name The name of the item
 * @return The created menu item
 */
auto ToolButton::registerPopupMenuEntry(const string& name, const string& iconName) -> GtkWidget* {
    if (this->popupMenu == nullptr) {
        // Todo (gtk4): create popovermenu (via ui file?)
        setPopupMenu(gtk_popover_menu_new_from_model(nullptr));
    }

    // GtkWidget* menuItem = nullptr;
    // if (iconName.empty()) {
    //     menuItem = gtk_check_button_new_with_label(name.c_str());
    // } else {
    //     menuItem = gtk_check_button_new();

    //     GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    //     gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));

    //     GtkWidget* label = gtk_label_new(name.c_str());
    //     gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    //     gtk_box_append(GTK_BOX(box), label);

    //     gtk_widget_set_child(menuItem, box);
    // }

    // gtk_check_button_set_draw_as_radio(GTK_CHECK_BUTTON(menuItem), true);
    // gtk_container_add(GTK_CONTAINER(this->popupMenu), menuItem);

    // return menuItem;
    return nullptr;
}

void ToolButton::updateDescription(const string& description) {
    this->description = description;
    gtk_widget_set_tooltip_text(item, description.c_str());
    if (GTK_IS_BUTTON(item)) {
        gtk_button_set_label(GTK_BUTTON(item), description.c_str());
    }
}

auto ToolButton::newItem() -> GtkWidget* {
    GtkWidget* it = nullptr;
    if (popupMenu) {
        it = gtk_button_new();
        gtk_menu_button_set_label(GTK_MENU_BUTTON(it), description.c_str());
        gtk_menu_button_set_child(GTK_MENU_BUTTON(it), gtk_image_new_from_icon_name(iconName.c_str()));
        gtk_menu_button_set_popover(GTK_MENU_BUTTON(it), popupMenu);
    } else if (group != GROUP_NOGROUP) {
        it = gtk_toggle_button_new();
        gtk_button_set_child(GTK_BUTTON(it), gtk_image_new_from_icon_name(iconName.c_str()));
        gtk_button_set_label(GTK_BUTTON(it), description.c_str());
    } else {
        it = gtk_button_new_from_icon_name(iconName.c_str());
        gtk_button_set_label(GTK_BUTTON(it), description.c_str());
    }
    gtk_widget_set_tooltip_text(it, description.c_str());
    return it;
}

auto ToolButton::getToolDisplayName() const -> string { return this->description; }

auto ToolButton::getNewToolIcon() const -> GtkWidget* { return gtk_image_new_from_icon_name(iconName.c_str()); }

auto ToolButton::getNewToolPixbuf() const -> GdkPixbuf* { return getPixbufFromImageIconName(); }


void ToolButton::setActive(bool active) {
    if (GTK_IS_TOGGLE_BUTTON(item)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(item), active);
    }
}
