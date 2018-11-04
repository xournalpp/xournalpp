#include "ToolSelectCombocontrol.h"

#include "ToolMenuHandler.h"
#include "gui/widgets/gtkmenutooltogglebutton.h"

#include <config.h>
#include <i18n.h>

ToolSelectCombocontrol::ToolSelectCombocontrol(ToolMenuHandler* th, ActionHandler* handler, GladeGui* gui, string id) :
		ToolButton(handler, gui, id, ACTION_TOOL_SELECT_RECT, GROUP_TOOL, true, "rect-select.svg", _("Select Rectangle"))
{

	XOJ_INIT_TYPE(ToolSelectCombocontrol);

	this->labelWidget = NULL;
	this->iconWidget = NULL;

	GtkWidget* popup = gtk_menu_new();

	GtkWidget* menuItem;

	this->iconSelectRect = gui->loadIconPixbuf("rect-select.svg");
	this->iconSelectRgion = gui->loadIconPixbuf("lasso.svg");
	this->iconSelectObject = gui->loadIconPixbuf("object-select.svg");
	this->iconPlayObject = gui->loadIconPixbuf("object-play.svg");
	g_object_ref(this->iconSelectRect);
	g_object_ref(this->iconSelectRgion);
	g_object_ref(this->iconSelectObject);
	g_object_ref(this->iconPlayObject);

	menuItem = gtk_image_menu_item_new_with_label(_C("Select Rectangle"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_RECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("rect-select.svg"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_C("Select Region"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_REGION, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("lasso.svg"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_C("Select Object"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("object-select.svg"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_C("Play Object"));
	gtk_container_add(GTK_CONTAINER(popup),menuItem);
	th->registerMenupoint(menuItem,ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("object-play.svg"));
	gtk_widget_show_all(menuItem);

	setPopupMenu(popup);
}

ToolSelectCombocontrol::~ToolSelectCombocontrol()
{
	XOJ_CHECK_TYPE(ToolSelectCombocontrol);

	g_object_unref(this->iconSelectRect);
	g_object_unref(this->iconSelectRgion);
	g_object_unref(this->iconSelectObject);
	g_object_unref(this->iconPlayObject);

	XOJ_RELEASE_TYPE(ToolSelectCombocontrol);
}

void ToolSelectCombocontrol::selected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(ToolSelectCombocontrol);

	if (this->item)
	{
		if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
		{
			g_warning("selected action %i which is not a toggle action! 2", action);
			return;
		}

		string description;

		if (action == ACTION_TOOL_SELECT_RECT && this->action != ACTION_TOOL_SELECT_RECT)
		{
			this->action = ACTION_TOOL_SELECT_RECT;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectRect);

			description = _("Select Rectangle");
		}
		else if (action == ACTION_TOOL_SELECT_REGION && this->action != ACTION_TOOL_SELECT_REGION)
		{
			this->action = ACTION_TOOL_SELECT_REGION;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectRgion);

			description = _("Select Region");
		}
		else if (action == ACTION_TOOL_SELECT_OBJECT && this->action != ACTION_TOOL_SELECT_OBJECT)
		{
			this->action = ACTION_TOOL_SELECT_OBJECT;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectObject);

			description = _("Select Object");
		}
		else if (action == ACTION_TOOL_PLAY_OBJECT && this->action != ACTION_TOOL_PLAY_OBJECT)
		{
			this->action = ACTION_TOOL_PLAY_OBJECT;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconPlayObject);

			description = _("Play Object");
		}
		gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());


		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action))
		{
			this->toolToggleButtonActive = (this->action == action);
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
		}
	}
}

GtkToolItem* ToolSelectCombocontrol::newItem()
{
	XOJ_CHECK_TYPE(ToolSelectCombocontrol);

	GtkToolItem* it;

	labelWidget = gtk_label_new(_C("Select Rectangle"));
	iconWidget = gtk_image_new_from_pixbuf(this->iconSelectRect);

	it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
	gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
	gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
	return it;
}
