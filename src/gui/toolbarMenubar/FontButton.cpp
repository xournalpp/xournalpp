#include "FontButton.h"

#include <config.h>
#include <i18n.h>

FontButton::FontButton(ActionHandler* handler, GladeGui* gui, string id, ActionType type, string description,
					   GtkWidget* menuitem) : AbstractToolItem(id, handler, type, menuitem)
{
	XOJ_INIT_TYPE(FontButton);

	this->gui = gui;
	this->description = description;
	this->fontButton = NULL;
}

FontButton::~FontButton()
{
	XOJ_RELEASE_TYPE(FontButton);
}

void FontButton::activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton)
{
	XOJ_CHECK_TYPE(FontButton);

	GtkFontButton* button = GTK_FONT_BUTTON(fontButton);

	string name = gtk_font_button_get_font_name(button);

	int pos = name.find_last_of(" ");
	this->font.setName(name.substr(0, pos));
	this->font.setSize(std::stod(name.substr(pos + 1)));

	handler->actionPerformed(ACTION_FONT_BUTTON_CHANGED, GROUP_NOGROUP, event, menuitem, NULL, true);
}

void FontButton::setFontFontButton(GtkWidget* fontButton, XojFont& font)
{
	GtkFontButton* button = GTK_FONT_BUTTON(fontButton);

	string txt = (bl::format("{1} {2}") % font.getName() % font.getSize()).str();

	gtk_font_button_set_font_name(button, txt.c_str());
}

void FontButton::setFont(XojFont& font)
{
	XOJ_CHECK_TYPE(FontButton);

	this->font = font;
	if (this->fontButton == NULL)
	{
		return;
	}

	setFontFontButton(this->fontButton, font);
}

XojFont FontButton::getFont()
{
	XOJ_CHECK_TYPE(FontButton);

	//essentially, copy the font object to prevent a memory leak.
	XojFont newfont;
	newfont.setName(font.getName());
	newfont.setSize(font.getSize());

	return newfont;
}

string FontButton::getToolDisplayName()
{
	XOJ_CHECK_TYPE(FontButton);

	return _("Font");
}

GtkWidget* FontButton::getNewToolIconImpl()
{
	XOJ_CHECK_TYPE(FontButton);

	return gtk_image_new_from_stock(GTK_STOCK_SELECT_FONT, GTK_ICON_SIZE_SMALL_TOOLBAR);
}

GtkToolItem* FontButton::createItem(bool horizontal)
{
	XOJ_CHECK_TYPE(FontButton);

	if (this->item)
	{
		return this->item;
	}

	this->item = newItem();
	g_object_ref(this->item);
	g_signal_connect(fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);
	return this->item;
}

GtkToolItem* FontButton::createTmpItem(bool horizontal)
{
	XOJ_CHECK_TYPE(FontButton);
	GtkWidget* fontButton = newFontButton();

	GtkToolItem* it = gtk_tool_item_new();

	gtk_container_add(GTK_CONTAINER(it), fontButton);
	gtk_tool_item_set_tooltip_text(it, this->description.c_str());
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(it), false);

	if (!this->font.getName().empty())
	{
		setFontFontButton(fontButton, this->font);
	}

	gtk_widget_show_all(GTK_WIDGET(it));
	return it;
}

void FontButton::showFontDialog()
{
	XOJ_CHECK_TYPE(FontButton);

	if (this->fontButton == NULL)
	{
		newItem();
	}

	gtk_button_clicked(GTK_BUTTON(this->fontButton));
}

GtkWidget* FontButton::newFontButton()
{
	XOJ_CHECK_TYPE(FontButton);
	GtkWidget* w = gtk_font_button_new();
	gtk_widget_show(w);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(w), TRUE);
	gtk_button_set_focus_on_click(GTK_BUTTON(w), FALSE);

	return w;
}

GtkToolItem* FontButton::newItem()
{
	XOJ_CHECK_TYPE(FontButton);

	if (this->fontButton)
	{
		g_object_unref(this->fontButton);
	}
	GtkToolItem* it = gtk_tool_item_new();

	this->fontButton = newFontButton();
	gtk_container_add(GTK_CONTAINER(it), this->fontButton);
	gtk_tool_item_set_tooltip_text(it, this->description.c_str());
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(it), false);

	g_signal_connect(this->fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);

	if (!this->font.getName().empty())
	{
		setFont(this->font);
	}

	return it;
}
