#include "ToolButton.h"

#include <utility>  // for move

#include <glib.h>
#include <gtk/gtk.h>

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "util/GVariantTemplate.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr


ToolButton::ToolButton(std::string id, Category cat, Action action, std::string iconName, std::string description,
                       bool toggle):
        AbstractToolItem(std::move(id), cat),
        iconName(std::move(iconName)),
        description(std::move(description)),
        action(action),
        toggle(toggle) {}

ToolButton::ToolButton(std::string id, Category cat, Action action, GVariant* target, std::string iconName,
                       std::string description):
        AbstractToolItem(std::move(id), cat),
        iconName(std::move(iconName)),
        description(std::move(description)),
        action(action),
        target(target, xoj::util::refsink),
        toggle(true) {}

auto ToolButton::createItem(ToolbarSide side) -> Widgetry {
    GtkWidget* btn = toggle ? gtk_toggle_button_new() : gtk_button_new();
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());
    gtk_widget_set_tooltip_text(btn, getToolDisplayName().c_str());
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), (std::string("win.") + Action_toString(action)).c_str());
    if (target) {
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
    }

    xoj::util::WidgetSPtr item;
    if (popoverFactory) {
        GtkMenuButton* menubutton = GTK_MENU_BUTTON(gtk_menu_button_new());
        gtk_menu_button_set_popover(menubutton, popoverFactory->createPopover());
        setMenuButtonDirection(menubutton, side);
        gtk_menu_button_set_always_show_arrow(menubutton, true);

        GtkBox* box = GTK_BOX(gtk_box_new(to_Orientation(side), 0));
        gtk_box_append(box, btn);
        gtk_box_append(box, GTK_WIDGET(menubutton));

        item.reset(GTK_WIDGET(box), xoj::util::adopt);
    } else {
        item.reset(btn, xoj::util::adopt);
    }

    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [this]() {
        GtkWidget* proxy;
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));

        if (toggle) {
            proxy = gtk_check_button_new();
#if GTK_CHECK_VERSION(4, 8, 0)
            gtk_check_button_set_child(GTK_CHECK_BUTTON(proxy), box);
#else
            gtk_check_button_set_label(GTK_CHECK_BUTTON(proxy), getToolDisplayName().c_str());
            g_object_unref(g_object_ref_sink(G_OBJECT(box)));
#endif
            // if (target) {
            //     gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(proxy), true);
            // }
        } else {
            proxy = gtk_button_new();
            gtk_button_set_child(GTK_BUTTON(proxy), box);
        }

        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy), (std::string("win.") + Action_toString(action)).c_str());
        if (target) {
            gtk_actionable_set_action_target_value(GTK_ACTIONABLE(proxy), target.get());
        }
        return proxy;
    };
    return {std::move(item), xoj::util::WidgetSPtr(createProxy(), xoj::util::adopt)};
}

auto ToolButton::getToolDisplayName() const -> std::string { return description; }

auto ToolButton::getNewToolIcon() const -> GtkWidget* { return gtk_image_new_from_icon_name(iconName.c_str()); }

void ToolButton::setPopoverFactory(const PopoverFactory* factory) { popoverFactory = factory; }
