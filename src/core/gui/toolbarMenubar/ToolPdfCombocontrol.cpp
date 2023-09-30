#include "ToolPdfCombocontrol.h"

#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "gui/toolbarMenubar/ToolButton.h"        // for ToolButton
#include "gui/widgets/gtkmenutooltogglebutton.h"  // for gtk_menu_tool_toggl...
#include "util/i18n.h"                            // for _

#include "ToolMenuHandler.h"  // for ToolMenuHandler

class ActionHandler;

using std::string;

ToolPdfCombocontrol::ToolPdfCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id):
        AbstractToolItem(std::move(id), handler, ACTION_TOOL_SELECT_PDF_TEXT_LINEAR),
        toolMenuHandler(toolMenuHandler),
        icon(toolMenuHandler->iconName("select-pdf-text-ht")),
        popover(gtk_menu_new(), xoj::util::adopt) {
    this->group = GROUP_TOOL;
    this->toolToggleOnlyEnable = true;

    addMenuitem(_("Select Linear PDF Text"), toolMenuHandler->iconName("select-pdf-text-ht"),
                ACTION_TOOL_SELECT_PDF_TEXT_LINEAR, GROUP_TOOL);
    addMenuitem(_("Select PDF Text In Rectangle"), toolMenuHandler->iconName("select-pdf-text-area"),
                ACTION_TOOL_SELECT_PDF_TEXT_RECT, GROUP_TOOL);
}

ToolPdfCombocontrol::~ToolPdfCombocontrol() = default;

void ToolPdfCombocontrol::addMenuitem(const string& text, const string& icon, ActionType type, ActionGroup group) {
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

void ToolPdfCombocontrol::selected(ActionGroup group, ActionType action) {
    if (this->item) {
        if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
            g_warning("selected action %i which is not a toggle action! 2", action);
            return;
        }

        string description;

        if (action == ACTION_TOOL_SELECT_PDF_TEXT_LINEAR && this->action != ACTION_TOOL_SELECT_PDF_TEXT_LINEAR) {
            this->action = ACTION_TOOL_SELECT_PDF_TEXT_LINEAR;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), toolMenuHandler->iconName("select-pdf-text-ht").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select PDF Text");
        } else if (action == ACTION_TOOL_SELECT_PDF_TEXT_RECT && this->action != ACTION_TOOL_SELECT_PDF_TEXT_RECT) {
            this->action = ACTION_TOOL_SELECT_PDF_TEXT_RECT;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget),
                                         toolMenuHandler->iconName("select-pdf-text-area").c_str(),
                                         GTK_ICON_SIZE_LARGE_TOOLBAR);

            description = _("Select PDF Area Text");
        }
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());


        if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action)) {
            this->toolToggleButtonActive = (this->action == action);
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
        }
    }
}

auto ToolPdfCombocontrol::newItem() -> GtkToolItem* {
    GtkToolItem* it = nullptr;

    labelWidget = gtk_label_new(_("Select Pdf Text"));
    iconWidget = gtk_image_new_from_icon_name(toolMenuHandler->iconName("select-pdf-text-ht").c_str(),
                                              GTK_ICON_SIZE_LARGE_TOOLBAR);

    it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
    gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
    gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popover.get());
    return it;
}

std::string ToolPdfCombocontrol::getToolDisplayName() const { return _("Select Linear PDF Text"); }
GtkWidget* ToolPdfCombocontrol::getNewToolIcon() const {
    return gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);
}
