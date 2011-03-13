#include "ToolSelectCombocontrol.h"
#include "../widgets/gtkmenutooltogglebutton.h"

#include "ToolMenuHandler.h"

#include <config.h>
#include <glib/gi18n-lib.h>

ToolSelectCombocontrol::ToolSelectCombocontrol(ToolMenuHandler * th, ActionHandler * handler, GladeGui * gui, String id) :
	ToolButton(handler, gui, id, ACTION_TOOL_SELECT_RECT, GROUP_TOOL, "rect-select.png", _("Select Rectangle")) {

	this->labelWidget = NULL;

	GtkWidget * popup = gtk_menu_new();

	GtkWidget * menuItem;

	this->iconSelectRect = gui->loadIconPixbuf("rect-select.png");
	this->iconSelectRgion = gui->loadIconPixbuf("lasso.png");
	this->iconSelectObject = gui->loadIconPixbuf("object-select.png");
	g_object_ref(this->iconSelectRect);
	g_object_ref(this->iconSelectRgion);
	g_object_ref(this->iconSelectObject);

	menuItem = gtk_image_menu_item_new_with_label(_("Select Rectangle"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_RECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("rect-select.png"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_("Select Region"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_REGION, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("lasso.png"));
	gtk_widget_show_all(menuItem);

	menuItem = gtk_image_menu_item_new_with_label(_("Select Object"));
	gtk_container_add(GTK_CONTAINER(popup), menuItem);
	th->registerMenupoint(menuItem, ACTION_TOOL_SELECT_OBJECT, GROUP_TOOL);
	gtk_image_menu_item_set_always_show_image(GTK_IMAGE_MENU_ITEM(menuItem), true);
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(menuItem), gui->loadIcon("object-select.png"));
	gtk_widget_show_all(menuItem);

	setPopupMenu(popup);
}

ToolSelectCombocontrol::~ToolSelectCombocontrol() {
	g_object_unref(this->iconSelectRect);
	g_object_unref(this->iconSelectRgion);
	g_object_unref(this->iconSelectObject);
}

void ToolSelectCombocontrol::selected(ActionGroup group, ActionType action) {
	if (item) {
		if (!GTK_IS_TOGGLE_TOOL_BUTTON(item)) {
			g_warning("selected action %i which is not a toggle action! 2", action);
			return;
		}

		const char * description = NULL;

		if (action == ACTION_TOOL_SELECT_RECT && this->action != ACTION_TOOL_SELECT_RECT) {
			this->action = ACTION_TOOL_SELECT_RECT;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectRect);

			description = _("Select Rectangle");
		} else if (action == ACTION_TOOL_SELECT_REGION && this->action != ACTION_TOOL_SELECT_REGION) {
			this->action = ACTION_TOOL_SELECT_REGION;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectRgion);

			description = _("Select Region");
		} else if (action == ACTION_TOOL_SELECT_OBJECT && this->action != ACTION_TOOL_SELECT_OBJECT) {
			this->action = ACTION_TOOL_SELECT_OBJECT;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconSelectObject);

			description = _("Select Object");
		}
		gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description);

		if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(item)) != (this->action == action)) {
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(item), (this->action == action));
		}
	}
}

GtkToolItem * ToolSelectCombocontrol::newItem() {
	GtkToolItem * it;

	labelWidget = gtk_label_new(_("Select Rectangle"));
	iconWidget = gtk_image_new_from_pixbuf(this->iconSelectRect);

	it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
	gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
	gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
	return it;
}
