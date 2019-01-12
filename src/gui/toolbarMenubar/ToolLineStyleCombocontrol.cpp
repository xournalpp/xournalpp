#include "ToolLineStyleCombocontrol.h"

#include "ToolMenuHandler.h"
#include "gui/widgets/gtkmenutooltogglebutton.h"

#include <config.h>
#include <i18n.h>

class ToolLineStyleType {
public:
	ToolLineStyleType(string name, string icon, ActionType type)
	 : name(name), icon(icon), type(type), pixbuf(NULL)
	{
	}
	~ToolLineStyleType()
	{
		g_object_unref(pixbuf);
		pixbuf = NULL;
	}

public:
	string name;
	string icon;
	ActionType type;
	GdkPixbuf* pixbuf;
};

ToolLineStyleCombocontrol::ToolLineStyleCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, GladeGui* gui, string id)
 : ToolButton(handler, gui, id, ACTION_TOOL_LINE_STYLE_PLAIN, GROUP_LINE_STYLE, false, "line-style-plain.svg", _("Line Style"))
{
	XOJ_INIT_TYPE(ToolLineStyleCombocontrol);

	this->toolMenuHandler = toolMenuHandler;
	this->labelWidget = NULL;
	this->iconWidget = NULL;
	setPopupMenu(gtk_menu_new());

	drawTypes.push_back(new ToolLineStyleType(_("Plain line"),		"line-style-plain.svg",		ACTION_TOOL_LINE_STYLE_PLAIN));
	drawTypes.push_back(new ToolLineStyleType(_("Dashed line"),		"line-style-dash.svg",		ACTION_TOOL_LINE_STYLE_DASH));
	drawTypes.push_back(new ToolLineStyleType(_("Dash-doted line"),	"line-style-dash-dot.svg",	ACTION_TOOL_LINE_STYLE_DASH_DOT));
	drawTypes.push_back(new ToolLineStyleType(_("Dotted line"),		"line-style-dot.svg",		ACTION_TOOL_LINE_STYLE_DOT));

	for (ToolLineStyleType* t : drawTypes)
	{
		createMenuItem(t->name, t->icon, t->type);
		t->pixbuf = gui->loadIconPixbuf(t->icon);
		g_object_ref(t->pixbuf);
	}
}

ToolLineStyleCombocontrol::~ToolLineStyleCombocontrol()
{
	XOJ_CHECK_TYPE(ToolLineStyleCombocontrol);

	for (ToolLineStyleType* t : drawTypes)
	{
		delete t;
	}
	this->drawTypes.clear();
	this->toolMenuHandler = NULL;

	XOJ_RELEASE_TYPE(ToolLineStyleCombocontrol);
}

void ToolLineStyleCombocontrol::createMenuItem(string name, string icon, ActionType type)
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

void ToolLineStyleCombocontrol::selected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(ToolLineStyleCombocontrol);

	if (!this->item)
	{
		return;
	}

	if (!GTK_IS_TOGGLE_TOOL_BUTTON(this->item))
	{
		g_warning("ToolLineStyleCombocontrol: selected action %i which is not a toggle action!", action);
		return;
	}

	string description;

	for (ToolLineStyleType* t : drawTypes)
	{
		if (action == t->type && this->action != t->type)
		{
			this->action = t->type;
			gtk_image_set_from_pixbuf(GTK_IMAGE(iconWidget), t->pixbuf);
			description = t->name;
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

GtkToolItem* ToolLineStyleCombocontrol::newItem()
{
	XOJ_CHECK_TYPE(ToolLineStyleCombocontrol);
	labelWidget = gtk_label_new(_("Line Style"));
	iconWidget = gtk_image_new_from_pixbuf(drawTypes[0]->pixbuf);

	GtkToolItem* it = gtk_menu_tool_toggle_button_new(iconWidget, _("Line Style"));
	gtk_tool_button_set_label_widget(GTK_TOOL_BUTTON(it), labelWidget);
	gtk_menu_tool_toggle_button_set_menu(GTK_MENU_TOOL_TOGGLE_BUTTON(it), popupMenu);
	return it;
}
