#include "ColorToolItem.h"

#include "model/ToolbarColorNames.h"
#include "gui/toolbarMenubar/ToolbarUtil.h"

#include <config.h>
#include <i18n.h>
#include <StringUtils.h>
#include <Util.h>

bool ColorToolItem::inUpdate = false;

ColorToolItem::ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, int color, bool selektor)
 : AbstractToolItem("", handler, selektor ? ACTION_SELECT_COLOR_CUSTOM : ACTION_SELECT_COLOR)
{
	XOJ_INIT_TYPE(ColorToolItem);

	this->color = color;
	this->toolHandler = toolHandler;
	this->group = GROUP_COLOR;
	this->iconWidget = NULL;
	this->parent = parent;

	updateName();
}

ColorToolItem::~ColorToolItem()
{
	XOJ_RELEASE_TYPE(ColorToolItem);

	if (this->iconWidget)
	{
		g_object_unref(this->iconWidget);
		this->iconWidget = NULL;
	}
}

bool ColorToolItem::isSelector()
{
	return this->action == ACTION_SELECT_COLOR_CUSTOM;
}

void ColorToolItem::updateName()
{
	if (this->action == ACTION_SELECT_COLOR_CUSTOM)
	{
		this->name = _("Select color");
	}
	else
	{
		this->name = ToolbarColorNames::getInstance().getColorName(this->color);
	}
}

void ColorToolItem::actionSelected(ActionGroup group, ActionType action)
{
	XOJ_CHECK_TYPE(ColorToolItem);

	inUpdate = true;
	if (this->group == group && this->item)
	{
		if (isSelector())
		{
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), isSelector());
		}
		enableColor(toolHandler->getColor());
	}
	inUpdate = false;
}

void ColorToolItem::enableColor(int color)
{
	XOJ_CHECK_TYPE(ColorToolItem);

	if (isSelector())
	{
		gtk_image_set_from_pixbuf(GTK_IMAGE(this->iconWidget), ToolbarUtil::newColorIconPixbuf(color, 16, !isSelector()));
		this->color = color;
		if (GTK_IS_TOGGLE_BUTTON(this->item))
		{
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), false);
		}
	}
	else
	{
		bool active = colorEqualsMoreOreLess(color);

		if (this->item)
		{
			gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON(this->item), active);
		}

		if (active)
		{
			this->toolHandler->setColorFound();

			// Only equals more ore less, so we will set it exact to the default color
			if (this->color != color)
			{
				this->toolHandler->setColor(this->color, true);
			}

		}
	}
}

int ColorToolItem::getColor()
{
	XOJ_CHECK_TYPE(ColorToolItem);

	return this->color;
}

string ColorToolItem::getId()
{
	XOJ_CHECK_TYPE(ColorToolItem);

	if (isSelector())
	{
		return "COLOR_SELECT";
	}

	string id = StringUtils::format("COLOR(0x%06x)", this->color);

	return id;
}

bool ColorToolItem::colorEqualsMoreOreLess(int color)
{
	XOJ_CHECK_TYPE(ColorToolItem);

	if (color == -1)
	{
		return false;
	}

	int r1 = (color & 0xff0000) >> 16;
	int g1 = (color & 0xff00) >> 8;
	int b1 = (color & 0xff);

	int r2 = (this->color & 0xff0000) >> 16;
	int g2 = (this->color & 0xff00) >> 8;
	int b2 = (this->color & 0xff);

	if (abs(r1 - r2) < 10 && abs(g1 - g2) < 10 && abs(b1 - b2) < 10)
	{
		return true;
	}
	return false;
}

/**
 * Show colochooser to select a custom color
 */
void ColorToolItem::showColorchooser()
{
	GtkWidget* dialog = gtk_color_chooser_dialog_new(_("Select color"), parent);
	gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(dialog), false);

	int response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_OK)
	{
		GdkRGBA color;
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), &color);
		this->color = (((int)(color.red * 255)) & 0xff) << 16 |
				(((int)(color.green * 255)) & 0xff) << 8 |
				(((int)(color.blue * 255)) & 0xff);
	}

	gtk_widget_destroy(dialog);
}

void ColorToolItem::activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton)
{
	XOJ_CHECK_TYPE(ColorToolItem);

	if (inUpdate)
	{
		return;
	}
	inUpdate = true;

	if (isSelector())
	{
		showColorchooser();
	}

	toolHandler->setColor(this->color, true);

	inUpdate = false;
}

GtkToolItem* ColorToolItem::newItem()
{
	XOJ_CHECK_TYPE(ColorToolItem);

	this->iconWidget = ToolbarUtil::newColorIcon(this->color, 16, !isSelector());
	GtkToolItem* it = gtk_toggle_tool_button_new();

	const gchar* name = this->name.c_str();
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM(it), name);
	gtk_tool_button_set_label(GTK_TOOL_BUTTON(it), name);

	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(it), this->iconWidget);

	return it;
}

string ColorToolItem::getToolDisplayName()
{
	XOJ_CHECK_TYPE(ColorToolItem);

	return this->name;
}

GtkWidget* ColorToolItem::getNewToolIcon()
{
	XOJ_CHECK_TYPE(ColorToolItem);

	return ToolbarUtil::newColorIcon(this->color, 16, !isSelector());
}
