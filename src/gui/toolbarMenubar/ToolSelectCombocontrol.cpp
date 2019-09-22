#include "ToolSelectCombocontrol.h"

#include "ToolMenuHandler.h"
#include "gui/widgets/gtkmenutooltogglebutton.h"

#include <config.h>
#include <i18n.h>

ToolSelectCombocontrol::ToolSelectCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id)
 : ToolButton(handler, id, ACTION_TOOL_SELECT_RECT, GROUP_TOOL, true, "rect-select", _("Select Rectangle")),
   toolMenuHandler(toolMenuHandler),
   popup(gtk_menu_new())
{
	addMenuitem(_("Select Rectangle"), "rect-select", ACTION_TOOL_SELECT_RECT, GROUP_TOOL);
	addMenuitem(_("Select Region"), "lasso", ACTION_TOOL_SELECT_REGION, GROUP_TOOL);
	addMenuitem(_("Select Object"), "object-select", ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL);
	addMenuitem(_("Play Object"), "object-play", ACTION_TOOL_PLAY_OBJECT, GROUP_TOOL);

	setPopupMenu(popup);
}

ToolSelectCombocontrol::~ToolSelectCombocontrol()
{
}

void ToolSelectCombocontrol::addMenuitem(string text, string icon, ActionType type, ActionGroup group)
{
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	GtkWidget* label = gtk_label_new(text.c_str());
	GtkWidget* menuItem = gtk_menu_item_new();

	gtk_container_add(GTK_CONTAINER(box), gtk_image_new_from_icon_name(icon.c_str(), GTK_ICON_SIZE_SMALL_TOOLBAR));
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_box_pack_end(GTK_BOX(box), label, true, true, 0);

	gtk_container_add(GTK_CONTAINER(menuItem), box);
	gtk_widget_show_all(menuItem);
	gtk_container_add(GTK_CONTAINER(popup), menuItem);

	toolMenuHandler->registerMenupoint(menuItem, type, group);
}

void ToolSelectCombocontrol::selected(ActionGroup group, ActionType action)
{
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
			gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), "rect-select", GTK_ICON_SIZE_SMALL_TOOLBAR);

			description = _("Select Rectangle");
		}
		else if (action == ACTION_TOOL_SELECT_REGION && this->action != ACTION_TOOL_SELECT_REGION)
		{
			this->action = ACTION_TOOL_SELECT_REGION;
			gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), "lasso", GTK_ICON_SIZE_SMALL_TOOLBAR);

			description = _("Select Region");
		}
		else if (action == ACTION_TOOL_SELECT_OBJECT && this->action != ACTION_TOOL_SELECT_OBJECT)
		{
			this->action = ACTION_TOOL_SELECT_OBJECT;
			gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), "object-select", GTK_ICON_SIZE_SMALL_TOOLBAR);

			description = _("Select Object");
		}
		else if (action == ACTION_TOOL_PLAY_OBJECT && this->action != ACTION_TOOL_PLAY_OBJECT)
		{
			this->action = ACTION_TOOL_PLAY_OBJECT;
			gtk_image_set_from_icon_name(GTK_IMAGE(iconWidget), "object-play", GTK_ICON_SIZE_SMALL_TOOLBAR);

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
	GtkToolItem* it;

	labelWidget = gtk_label_new(_("Select Rectangle"));
	iconWidget = gtk_image_new_from_icon_name("rect-select", GTK_ICON_SIZE_SMALL_TOOLBAR);

	it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
	gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
	gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
	return it;
}
