#include "ToolButton.h"

#include <utility>  // for move

#include <glib.h>
#include <gtk/gtk.h>

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "util/GVariantTemplate.h"
#include "util/GtkUtil.h"
#include "util/gtk4_helper.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr
#include "util/raii/GVariantSPtr.h"


ToolButton::ToolButton(std::string id, Action action, std::string iconName, std::string description, bool toggle):
        AbstractToolItem(id),
        iconName(std::move(iconName)),
        description(std::move(description)),
        action(action),
        toggle(toggle) {}

ToolButton::ToolButton(std::string id, Action action, GVariant* target, std::string iconName, std::string description):
        AbstractToolItem(id),
        iconName(std::move(iconName)),
        description(std::move(description)),
        action(action),
        target(target, xoj::util::refsink),
        toggle(true) {}

ToolButton::~ToolButton() = default;

void ToolButton::updateDescription(const std::string& description) {
    this->description = description;
    gtk_widget_set_tooltip_text(this->item.get(), description.c_str());
}

auto ToolButton::createItem(bool horizontal) -> GtkWidget* {
    GtkWidget* btn = toggle ? gtk_toggle_button_new() : gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());
    gtk_widget_set_tooltip_text(btn, getToolDisplayName().c_str());
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), (std::string("win.") + Action_toString(action)).c_str());
    if (target) {
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
        xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));
    }

    if (popover) {
        GtkMenuButton* menubutton = GTK_MENU_BUTTON(gtk_menu_button_new());
        gtk_menu_button_set_popover(menubutton, popover.get());
        gtk_menu_button_set_direction(menubutton, GTK_ARROW_DOWN);  // TODO: fix directions

        GtkBox* box = GTK_BOX(gtk_box_new(horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0));
        gtk_box_append(box, btn);
        gtk_box_append(box, GTK_WIDGET(menubutton));

        this->item.reset(GTK_WIDGET(box), xoj::util::adopt);
    } else {
        this->item.reset(btn, xoj::util::adopt);
    }
    return this->item.get();
}

auto ToolButton::getToolDisplayName() const -> std::string { return this->description; }

auto ToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);
}

void ToolButton::setPopover(GtkWidget* popover) { this->popover.reset(popover, xoj::util::refsink); }
