#include "ToolDrawCombocontrol.h"

#include "ToolMenuHandler.h"
#include "gui/widgets/gtkmenutooltogglebutton.h"

#include <config.h>
#include <i18n.h>

ToolDrawCombocontrol::ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, GladeGui* gui, string id)
 : ToolButton(handler, gui, id, ACTION_TOOL_DRAW_RECT, GROUP_RULER, false, "rect-draw.png", _("Draw Rectangle"))
{
	XOJ_INIT_TYPE(ToolDrawCombocontrol);

	this->toolMenuHandler = toolMenuHandler;
	this->labelWidget = NULL;
	this->iconWidget = NULL;

	this->iconDrawRect = gui->loadIconPixbuf("rect-draw.svg");
	g_object_ref(this->iconDrawRect);

	this->iconDrawCirc = gui->loadIconPixbuf("circle-draw.svg");
	g_object_ref(this->iconDrawCirc);

	this->iconDrawArr = gui->loadIconPixbuf("arrow-draw.svg");
	g_object_ref(this->iconDrawArr);

	this->iconDrawLine = gui->loadIconPixbuf("ruler.svg");
	g_object_ref(this->iconDrawLine);

	this->iconAutoDrawLine = gui->loadIconPixbuf("shape_recognizer.svg");
	g_object_ref(this->iconAutoDrawLine);

	setPopupMenu(gtk_menu_new());

	createMenuItem(_("Draw Rectangle"), "rect-draw.svg", ACTION_TOOL_DRAW_RECT);
	createMenuItem(_C("Draw Circle"), "circle-draw.svg", ACTION_TOOL_DRAW_CIRCLE);
	createMenuItem(_C("Draw Arrow"), "arrow-draw.svg", ACTION_TOOL_DRAW_ARROW);
	createMenuItem(_C("Draw Line"), "ruler.svg", ACTION_RULER);
	createMenuItem(_C("Recognize Lines"), "shape_recognizer.svg", ACTION_SHAPE_RECOGNIZER);
}

ToolDrawCombocontrol::~ToolDrawCombocontrol()
{
	XOJ_CHECK_TYPE(ToolDrawCombocontrol);

	g_object_unref(this->iconDrawRect);
	this->iconDrawRect = NULL;

	g_object_unref(this->iconDrawCirc);
	this->iconDrawCirc = NULL;

	g_object_unref(this->iconDrawArr);
	this->iconDrawArr = NULL;

	g_object_unref(this->iconDrawLine);
	this->iconDrawLine = NULL;

	g_object_unref(this->iconAutoDrawLine);
	this->iconAutoDrawLine = NULL;

	XOJ_RELEASE_TYPE(ToolDrawCombocontrol);
}

void ToolDrawCombocontrol::createMenuItem(string name, string icon, ActionType type)
{
	GtkWidget* menuItem =  gtk_menu_item_new ();
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_add(GTK_CONTAINER(box), gui->loadIcon(icon));
	gtk_container_add(GTK_CONTAINER(box), gtk_label_new(name.c_str()));
	gtk_container_add(GTK_CONTAINER(menuItem), box);

	gtk_container_add(GTK_CONTAINER(popupMenu), menuItem);
	toolMenuHandler->registerMenupoint(menuItem, type, GROUP_RULER);
	gtk_widget_show_all(menuItem);
}

void ToolDrawCombocontrol::selected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(ToolDrawCombocontrol);

	if (!this->item)
	{
		return;
	}

	if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
	{
		g_warning("selected action %i which is not a toggle action! 2", action);
		return;
	}

	string description;

	if (action == ACTION_TOOL_DRAW_RECT && this->action != ACTION_TOOL_DRAW_RECT)
	{
		this->action = ACTION_TOOL_DRAW_RECT;
		gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawRect);

		description = _("Draw Rectangle");
	}
	else if (action == ACTION_TOOL_DRAW_CIRCLE && this->action != ACTION_TOOL_DRAW_CIRCLE)
	{
		this->action = ACTION_TOOL_DRAW_CIRCLE;
		gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawCirc);

		description = _("Draw Circle");
	}
	else if (action == ACTION_TOOL_DRAW_ARROW && this->action != ACTION_TOOL_DRAW_ARROW)
	{
		this->action = ACTION_TOOL_DRAW_ARROW;
		gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawArr);

		description = _("Draw Arrow");
	}
	else if (action == ACTION_RULER && this->action != ACTION_RULER)
	{
		this->action = ACTION_RULER;
		gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconDrawLine);

		description = _("Draw Line");
	}
	else if (action == ACTION_SHAPE_RECOGNIZER && this->action != ACTION_SHAPE_RECOGNIZER)
	{
		this->action = ACTION_SHAPE_RECOGNIZER;
		gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->iconAutoDrawLine);

		description = _("Recognize Lines");
	}
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(item), description.c_str());


	if (gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(this->item)) != (this->action == action))
	{
		this->toolToggleButtonActive = (this->action == action);
		gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), this->toolToggleButtonActive);
	}
}

GtkToolItem* ToolDrawCombocontrol::newItem()
{
	XOJ_CHECK_TYPE(ToolDrawCombocontrol);

	labelWidget = gtk_label_new(_C("Draw Rectangle"));
	iconWidget = gtk_image_new_from_pixbuf(this->iconDrawRect);

	GtkToolItem* it = gtk_menu_tool_toggle_button_new(iconWidget, "test0");
	gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
	gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
	return it;
}
