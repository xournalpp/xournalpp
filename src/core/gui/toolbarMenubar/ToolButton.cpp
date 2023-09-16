#include "ToolButton.h"

#include <utility>  // for move

#include <glib.h>
#include <gtk/gtk.h>

#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "util/GVariantTemplate.h"
#include "util/GtkUtil.h"
#include "util/gtk4_helper.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr


ToolButton::ToolButton(std::string id, Action action, std::string iconName, std::string description, bool toggle):
        AbstractToolItem(id),
        iconName(std::move(iconName)),
        description(std::move(description)),
        newAction(action),
        toggle(toggle) {}

ToolButton::ToolButton(std::string id, Action action, GVariant* target, std::string iconName, std::string description):
        AbstractToolItem(id),
        iconName(std::move(iconName)),
        description(std::move(description)),
        newAction(action),
        target(target, xoj::util::refsink),
        toggle(true) {}

ToolButton::~ToolButton() = default;

void ToolButton::updateDescription(const std::string& description) {
    this->description = description;
    gtk_widget_set_tooltip_text(this->item.get(), description.c_str());
}

auto ToolButton::createItem(bool horizontal) -> GtkWidget* {
    GtkWidget* btn = toggle ? gtk_toggle_button_new() : gtk_button_new();
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());
    gtk_widget_set_tooltip_text(btn, getToolDisplayName().c_str());
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), (std::string("win.") + Action_toString(newAction)).c_str());
    if (target) {
        gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
        xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));
    }

    // For the sake of deprecated GtkToolbar, wrap the button in a GtkToolItem
    // Todo(gtk4): remove
    GtkToolItem* it = gtk_tool_item_new();
    if (popover) {
        GtkMenuButton* menubutton = GTK_MENU_BUTTON(gtk_menu_button_new());
        gtk_widget_set_can_focus(GTK_WIDGET(menubutton), false);  // todo(gtk4) not necessary anymore
        gtk_menu_button_set_popover(menubutton, popover.get());
        gtk_menu_button_set_direction(menubutton, GTK_ARROW_DOWN);

        if (target && toggle) {
            // Pressing the menubutton also selects the tool
#if GTK_MAJOR_VERSION == 3
            g_signal_connect_object(menubutton, "clicked", G_CALLBACK(+[](GtkButton*, gpointer tglbtn) {
                                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tglbtn), true);
                                    }),
                                    btn, GConnectFlags(0));
#else
            g_signal_connect_object(menubutton, "activate", G_CALLBACK(+[](GtkMenuButton*, gpointer tglbtn) {
                                        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tglbtn), true);
                                    }),
                                    btn, GConnectFlags(0));
#endif
        }

        GtkBox* box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
        gtk_box_append(box, btn);
        gtk_box_append(box, GTK_WIDGET(menubutton));

        gtk_container_add(GTK_CONTAINER(it), GTK_WIDGET(box));

        g_signal_connect_object(it, "toolbar-reconfigured", G_CALLBACK(+[](GtkToolItem* it, gpointer box) {
                                    gtk_orientable_set_orientation(GTK_ORIENTABLE(box),
                                                                   gtk_tool_item_get_orientation(it));
                                }),
                                box, GConnectFlags(0));
        g_signal_connect_object(it, "toolbar-reconfigured", G_CALLBACK(+[](GtkToolItem* it, gpointer btn) {
                                    const bool h = gtk_tool_item_get_orientation(it) == GTK_ORIENTATION_HORIZONTAL;
                                    gtk_menu_button_set_direction(GTK_MENU_BUTTON(btn),
                                                                  h ? GTK_ARROW_DOWN : GTK_ARROW_RIGHT);
                                }),
                                menubutton, GConnectFlags(0));

    } else {
        gtk_container_add(GTK_CONTAINER(it), GTK_WIDGET(btn));
    }

    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [this]() {
        GtkWidget* proxy;
        if (toggle) {
            proxy = gtk_check_menu_item_new();
            if (target) {
                gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(proxy), true);
            }
        } else {
            proxy = gtk_menu_item_new();
        }

        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));

        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(newAction)).c_str());
        if (target) {
            gtk_actionable_set_action_target_value(GTK_ACTIONABLE(proxy), target.get());
        }
        xoj::util::gtk::fixActionableInitialSensitivity(GTK_ACTIONABLE(proxy));
        return proxy;
    };
    gtk_tool_item_set_proxy_menu_item(it, "", createProxy());
    this->item.reset(GTK_WIDGET(it), xoj::util::adopt);
    return this->item.get();
}

auto ToolButton::getToolDisplayName() const -> std::string { return this->description; }

auto ToolButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(iconName.c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);
}

void ToolButton::setPopover(GtkWidget* popover) { this->popover.reset(popover, xoj::util::refsink); }
