#include "ToolDrawCombocontrol.h"
#include "../widgets/gtkmenutooltogglebutton.h"

#include "ToolMenuHandler.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ToolDrawCombocontrol::ToolDrawCombocontrol(ToolMenuHandler* th,
                                           ActionHandler* handler, GladeGui* gui, string id) :
ToolButton(handler, gui, id, ACTION_TOOL_DRAW_RECT, GROUP_RULER, false,
           "rect-draw.png", _("Draw Rectangle"))
{

    XOJ_INIT_TYPE(ToolDrawCombocontrol);

    this->labelWidget = NULL;

    GtkWidget* popup = gtk_menu_new();

    GtkWidget* menuItem;

    this->iconDrawRect = gui->loadIconPixbuf("rect-draw.png");
    this->iconDrawCirc = gui->loadIconPixbuf("circle-draw.png");
    this->iconDrawArr = gui->loadIconPixbuf("arrow-draw.png");
    this->iconDrawLine = gui->loadIconPixbuf("ruler.png");
    this->iconAutoDrawLine = gui->loadIconPixbuf("shape_recognizer.png");
    g_object_ref(this->iconDrawRect);
    g_object_ref(this->iconDrawCirc);
    g_object_ref(this->iconDrawArr);
    g_object_ref(this->iconDrawLine);
    g_object_ref(this->iconAutoDrawLine);

    menuItem = gtk_image_menu_item_new_with_label(_("Draw Rectangle"));
    gtk_container_add(GTK_CONTAINER(popup), menuItem);
    th->registerMenupoint(menuItem, ACTION_TOOL_DRAW_RECT, GROUP_RULER);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem),
                                  gui->loadIcon("rect-draw.png"));
    gtk_widget_show_all(menuItem);

    menuItem = gtk_image_menu_item_new_with_label(_("Draw Circle"));
    gtk_container_add(GTK_CONTAINER(popup), menuItem);
    th->registerMenupoint(menuItem, ACTION_TOOL_DRAW_CIRCLE, GROUP_RULER);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem),
                                  gui->loadIcon("circle-draw.png"));
    gtk_widget_show_all(menuItem);

    menuItem = gtk_image_menu_item_new_with_label(_("Draw Arrow"));
    gtk_container_add(GTK_CONTAINER(popup), menuItem);
    th->registerMenupoint(menuItem, ACTION_TOOL_DRAW_ARROW, GROUP_RULER);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem),
                                  gui->loadIcon("arrow-draw.png"));
    gtk_widget_show_all(menuItem);

    menuItem = gtk_image_menu_item_new_with_label(_("Draw Line"));
    gtk_container_add(GTK_CONTAINER(popup), menuItem);
    th->registerMenupoint(menuItem, ACTION_RULER, GROUP_RULER);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem),
                                  gui->loadIcon("ruler.png"));
    gtk_widget_show_all(menuItem);

    menuItem = gtk_image_menu_item_new_with_label(_("Recognize Lines"));
    gtk_container_add(GTK_CONTAINER(popup), menuItem);
    th->registerMenupoint(menuItem, ACTION_SHAPE_RECOGNIZER, GROUP_SHAPE_RECOGNIZER);
    gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem),
                                  gui->loadIcon("shape_recognizer.png"));
    gtk_widget_show_all(menuItem);


    setPopupMenu(popup);
}

ToolDrawCombocontrol::~ToolDrawCombocontrol()
{
    XOJ_CHECK_TYPE(ToolDrawCombocontrol);

    g_object_unref(this->iconDrawRect);
    g_object_unref(this->iconDrawCirc);
    g_object_unref(this->iconDrawArr);
    g_object_unref(this->iconDrawLine);
    g_object_unref(this->iconAutoDrawLine);

    XOJ_RELEASE_TYPE(ToolDrawCombocontrol);
}

void ToolDrawCombocontrol::selected(ActionGroup group, ActionType action)
{
    XOJ_CHECK_TYPE(ToolDrawCombocontrol);

    if (this->item)
    {
        if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
        {
            g_warning("selected action %i which is not a toggle action! 2", action);
            return;
        }

        const char* description = NULL;

        if (action == ACTION_TOOL_DRAW_RECT &&
            this->action != ACTION_TOOL_DRAW_RECT)
        {
            this->action = ACTION_TOOL_DRAW_RECT;
            gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawRect);

            description = _("Draw Rectangle");
        }
        else if (action == ACTION_TOOL_DRAW_CIRCLE &&
                 this->action != ACTION_TOOL_DRAW_CIRCLE)
        {
            this->action = ACTION_TOOL_DRAW_CIRCLE;
            gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawCirc);

            description = _("Draw Circle");
        }
        else if (action == ACTION_TOOL_DRAW_ARROW &&
                 this->action != ACTION_TOOL_DRAW_ARROW)
        {
            this->action = ACTION_TOOL_DRAW_ARROW;
            gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawArr);

            description = _("Draw Arrow");
        }
        else if (action == ACTION_RULER &&
                 this->action != ACTION_RULER)
        {
            this->action = ACTION_RULER;
            gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawLine);

            description = _("Draw Line");
        }
        else if (action == ACTION_SHAPE_RECOGNIZER &&
                 this->action != ACTION_SHAPE_RECOGNIZER)
        {
            this->action = ACTION_SHAPE_RECOGNIZER;
            gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconAutoDrawLine);

            description = _("Recognize Lines");
        }
        gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description);


        if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) !=
            (this->action == action))
        {
            this->toolToggleButtonActive = (this->action == action);
            gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item),
                                              this->toolToggleButtonActive);
        }
    }
}

GtkToolItem* ToolDrawCombocontrol::newItem()
{
    XOJ_CHECK_TYPE(ToolDrawCombocontrol);

    GtkToolItem* it;

    labelWidget = gtk_label_new(_("Draw Rectangle"));
    iconWidget = gtk_image_new_from_pixbuf(this->iconDrawRect);

    it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
    gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
    gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it),
                                         popupMenu);
    return it;
}
