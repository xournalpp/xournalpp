#include "ToolDrawCombocontrol.h"

#include "ToolMenuHandler.h"
#include "gui/widgets/gtkmenutooltogglebutton.h"

#include <config.h>
#include <i18n.h>

struct ToolDrawType {
	string name;
	string icon;
	ActionType type;
};

ToolDrawType types[ToolDrawCombocontrol_EntryCount] = {
	{ .name = _("Draw Rectangle"),  .icon = "rect-draw.svg",        .type = ACTION_TOOL_DRAW_RECT   },
	{ .name = _("Draw Circle"),     .icon = "circle-draw.svg",      .type = ACTION_TOOL_DRAW_CIRCLE },
	{ .name = _("Draw Arrow"),      .icon = "arrow-draw.svg",       .type = ACTION_TOOL_DRAW_ARROW  },
	{ .name = _("Draw Line"),       .icon = "ruler.svg",            .type = ACTION_RULER            },
	{ .name = _("Recognize Lines"), .icon = "shape_recognizer.svg", .type = ACTION_SHAPE_RECOGNIZER }
};

ToolDrawCombocontrol::ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, GladeGui* gui, string id)
 : ToolButton(handler, gui, id, ACTION_TOOL_DRAW_RECT, GROUP_RULER, false, "rect-draw.png", _("Draw Rectangle"))
{
	XOJ_INIT_TYPE(ToolDrawCombocontrol);

	this->toolMenuHandler = toolMenuHandler;
	this->labelWidget = NULL;
	this->iconWidget = NULL;
	setPopupMenu(gtk_menu_new());

	for (int i = 0; i < ToolDrawCombocontrol_EntryCount; i++)
	{
		ToolDrawType& t = types[i];
		createMenuItem(t.name, t.icon, t.type);

		this->icons[i] = gui->loadIconPixbuf(t.icon);
		g_object_ref(this->icons[i]);
	}
}

ToolDrawCombocontrol::~ToolDrawCombocontrol()
{
	XOJ_CHECK_TYPE(ToolDrawCombocontrol);

	for (int i = 0; i < ToolDrawCombocontrol_EntryCount; i++)
	{
		g_object_unref(icons[i]);
		icons[i] = NULL;
	}

	this->toolMenuHandler = NULL;

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
		g_warning("ToolDrawCombocontrol: selected action %i which is not a toggle action!", action);
		return;
	}

	string description;

	for (int i = 0; i < ToolDrawCombocontrol_EntryCount; i++)
	{
		ToolDrawType& t = types[i];

		if (action == t.type && this->action != t.type)
		{
			this->action = t.type;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), this->icons[i]);
			description = t.name;
			break;
		}
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
	iconWidget = gtk_image_new_from_pixbuf(this->icons[0]);

	GtkToolItem* it = gtk_menu_tool_toggle_button_new(iconWidget, _C("Draw Rectangle"));
	gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
	gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
	return it;
}
