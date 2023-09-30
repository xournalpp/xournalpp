#include "ToolDrawCombocontrol.h"

#include <utility>  // for move

#include <glib.h>  // for g_warning

#include "gui/toolbarMenubar/ToolButton.h"        // for ToolButton
#include "gui/widgets/gtkmenutooltogglebutton.h"  // for gtk_menu_tool_toggl...
#include "util/i18n.h"                            // for _

#include "ToolMenuHandler.h"  // for ToolMenuHandler

class ActionHandler;

using std::string;

class ToolDrawType {
public:
    ToolDrawType(string name, string icon, ActionType type): name(std::move(name)), icon(std::move(icon)), type(type) {}


    string name;
    string icon;
    ActionType type;
};

ToolDrawCombocontrol::ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id):
        AbstractToolItem(std::move(id), handler, ACTION_TOOL_DRAW_RECT),
        toolMenuHandler(toolMenuHandler),
        icon(toolMenuHandler->iconName("combo-drawing-type")),
        popover(gtk_menu_new(), xoj::util::adopt) {
    this->group = GROUP_RULER;

    drawTypes.push_back(
            new ToolDrawType(_("Draw Rectangle"), toolMenuHandler->iconName("draw-rect"), ACTION_TOOL_DRAW_RECT));
    drawTypes.push_back(
            new ToolDrawType(_("Draw Ellipse"), toolMenuHandler->iconName("draw-ellipse"), ACTION_TOOL_DRAW_ELLIPSE));
    drawTypes.push_back(
            new ToolDrawType(_("Draw Arrow"), toolMenuHandler->iconName("draw-arrow"), ACTION_TOOL_DRAW_ARROW));
    drawTypes.push_back(new ToolDrawType(_("Draw Double Arrow"), toolMenuHandler->iconName("draw-double-arrow"),
                                         ACTION_TOOL_DRAW_DOUBLE_ARROW));
    drawTypes.push_back(new ToolDrawType(_("Draw Line"), toolMenuHandler->iconName("draw-line"), ACTION_RULER));
    drawTypes.push_back(new ToolDrawType(_("Draw coordinate system"),
                                         toolMenuHandler->iconName("draw-coordinate-system"),
                                         ACTION_TOOL_DRAW_COORDINATE_SYSTEM));
    drawTypes.push_back(
            new ToolDrawType(_("Draw Spline"), toolMenuHandler->iconName("draw-spline"), ACTION_TOOL_DRAW_SPLINE));
    drawTypes.push_back(new ToolDrawType(_("Stroke recognizer"), toolMenuHandler->iconName("shape-recognizer"),
                                         ACTION_SHAPE_RECOGNIZER));

    for (ToolDrawType* t: drawTypes) { createMenuItem(t->name, t->icon, t->type); }
}

ToolDrawCombocontrol::~ToolDrawCombocontrol() {
    for (ToolDrawType* t: drawTypes) { delete t; }
    this->drawTypes.clear();
    this->toolMenuHandler = nullptr;
}

void ToolDrawCombocontrol::createMenuItem(const string& name, const string& icon, ActionType type) {
    GtkWidget* menuItem = gtk_menu_item_new();
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_container_add(GTK_CONTAINER(box), gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_container_add(GTK_CONTAINER(box), gtk_label_new(name.c_str()));
    gtk_container_add(GTK_CONTAINER(menuItem), box);

    gtk_container_add(GTK_CONTAINER(popover.get()), menuItem);
    toolMenuHandler->registerMenupoint(menuItem, type, GROUP_RULER);
    gtk_widget_show_all(menuItem);
}

void ToolDrawCombocontrol::selected(ActionGroup group, ActionType action) {
    if (!this->item) {
        return;
    }

    if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item)) {
        g_warning("ToolDrawCombocontrol: selected action %i which is not a toggle action!", action);
        return;
    }

    string description;

    for (ToolDrawType* t: drawTypes) {
        if (action == t->type && this->action != t->type) {
            this->action = t->type;
            gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), t->icon.c_str(), GTK_ICON_SIZE_LARGE_TOOLBAR);
            description = t->name;
            break;
        }
    }

    gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());
    if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action)) {
        this->toolToggleButtonActive = (this->action == action);
        gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
    }
}

auto ToolDrawCombocontrol::newItem() -> GtkToolItem* {
    labelWidget = gtk_label_new(_("Draw Rectangle"));
    iconWidget = gtk_image_new_from_icon_name(drawTypes[0]->icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);

    GtkToolItem* it = gtk_menu_tool_toggle_button_new(iconWidget, _("Draw Rectangle"));
    gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
    gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popover.get());
    return it;
}

std::string ToolDrawCombocontrol::getToolDisplayName() const { return _("Drawing Type Combo"); }
GtkWidget* ToolDrawCombocontrol::getNewToolIcon() const {
    return gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR);
}
