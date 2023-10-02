#include "ToolSelectCombocontrol.h"

#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "gui/toolbarMenubar/ToolButton.h"        // for ToolButton
#include "gui/widgets/gtkmenutooltogglebutton.h"  // for gtk_menu_tool_toggl...
#include "util/i18n.h"                            // for _

#include "ToolMenuHandler.h"  // for ToolMenuHandler

class ActionHandler;

using std::string;

ToolSelectCombocontrol::ToolSelectCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id,
                                               bool hideAudio):
        AbstractToolItem(std::move(id), handler, ACTION_TOOL_SELECT_RECT),
        toolMenuHandler(toolMenuHandler),
        icon(toolMenuHandler->iconName("combo-selection")),
        popover(gtk_menu_new(), xoj::util::adopt) {
    this->group = GROUP_TOOL;
    this->toolToggleOnlyEnable = true;

    addMenuitem(_("Select Rectangle"), toolMenuHandler->iconName("select-rect"), ACTION_TOOL_SELECT_RECT, GROUP_TOOL);
    addMenuitem(_("Select Region"), toolMenuHandler->iconName("select-lasso"), ACTION_TOOL_SELECT_REGION, GROUP_TOOL);
    addMenuitem(_("Select Multi-Layer Rectangle"), toolMenuHandler->iconName("select-multilayer-rect"), ACTION_TOOL_SELECT_MULTILAYER_RECT, GROUP_TOOL);
    addMenuitem(_("Select Multi-Layer Region"), toolMenuHandler->iconName("select-multilayer-lasso"), ACTION_TOOL_SELECT_MULTILAYER_REGION, GROUP_TOOL);
    addMenuitem(_("Select Object"), toolMenuHandler->iconName("object-select"), ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL);
    if (!hideAudio) {
        addMenuitem(_("Play Object"), toolMenuHandler->iconName("object-play"), ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL);
    }
}

ToolSelectCombocontrol::~ToolSelectCombocontrol() = default;

void ToolSelectCombocontrol::addMenuitem(const string& text, const string& icon, ActionType type, ActionGroup group) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget* label = gtk_label_new(text.c_str());
    GtkWidget* menuItem = gtk_menu_item_new();

    gtk_container_add(GTK_CONTAINER(box), gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_box_pack_end(GTK_BOX(box), label, true, true, 0);

    gtk_container_add(GTK_CONTAINER(menuItem), box);
    gtk_widget_show_all(menuItem);
    gtk_container_add(GTK_CONTAINER(popover.get()), menuItem);

    toolMenuHandler->registerMenupoint(menuItem, type, group);
}

void ToolSelectCombocontrol::selected(ActionGroup group, ActionType action) {
    if (this->item) {
        if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
            g_warning("selected action %i which is not a toggle action! 2", action);
            return;
        }

        string description;

        if (action == ACTION_TOOL_SELECT_RECT && this->action != ACTION_TOOL_SELECT_RECT) {
            this->action = ACTION_TOOL_SELECT_RECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("select-rect").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select Rectangle");
        } else if (action == ACTION_TOOL_SELECT_REGION && this->action != ACTION_TOOL_SELECT_REGION) {
            this->action = ACTION_TOOL_SELECT_REGION;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("select-lasso").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select Region");
        } else if (action == ACTION_TOOL_SELECT_MULTILAYER_RECT && this->action != ACTION_TOOL_SELECT_MULTILAYER_RECT) {
            this->action = ACTION_TOOL_SELECT_MULTILAYER_RECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget),
                                         toolMenuHandler->iconName("select-multilayer-rect").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select Multi-Layer Rectangle");
        } else if (action == ACTION_TOOL_SELECT_MULTILAYER_REGION && this->action != ACTION_TOOL_SELECT_MULTILAYER_REGION) {
            this->action = ACTION_TOOL_SELECT_MULTILAYER_REGION;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget),
                                         toolMenuHandler->iconName("select-multilayer-lasso").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select Multi-Layer Region");
        } else if (action == ACTION_TOOL_SELECT_OBJECT && this->action != ACTION_TOOL_SELECT_OBJECT) {
            this->action = ACTION_TOOL_SELECT_OBJECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("object-select").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select Object");
        } else if (action == ACTION_TOOL_PLAY_OBJECT && this->action != ACTION_TOOL_PLAY_OBJECT) {
            this->action = ACTION_TOOL_PLAY_OBJECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("object-play").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Play Object");
        }
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());


        if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action)) {
            this->toolToggleButtonActive = (this->action == action);
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
        }
    }
}

auto ToolSelectCombocontrol::newItem() -> GtkToolItem* {
    GtkToolItem* it = nullptr;

    labelWidget = gtk_label_new(_("Select Rectangle"));
    iconWidget =
            gtk_image_new_from_icon_name(toolMenuHandler->iconName("select-rect").c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);

    it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
    gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
    gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popover.get());
    return it;
}

std::string ToolSelectCombocontrol::getToolDisplayName() const { return _("Selection Combo"); }
GtkWidget* ToolSelectCombocontrol::getNewToolIcon() const {
    return gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);
}
