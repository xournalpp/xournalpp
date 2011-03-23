#include "FontButton.h"

#include <stdlib.h>
// TODO: AA: type check

FontButton::FontButton(ActionHandler * handler, GladeGui * gui, String id, ActionType type, String description, GtkWidget * menuitem) :
	AbstractToolItem(id, handler, type, menuitem) {
	this->gui = gui;
	this->description = description;
	fontButton = NULL;
}

FontButton::~FontButton() {
}

void FontButton::activated(GdkEvent *event, GtkMenuItem *menuitem, GtkToolButton *toolbutton) {
	GtkFontButton * button = GTK_FONT_BUTTON(fontButton);

	String name = gtk_font_button_get_font_name(button);

	int pos = name.lastIndexOf(" ");
	this->font.setName(name.substring(0, pos));
	this->font.setSize(atof(name.substring(pos + 1).c_str()));

	handler->actionPerformed(ACTION_SELECT_FONT, GROUP_NOGROUP, event, menuitem, NULL, true);
}

void FontButton::setFont(XojFont & font) {
	this->font = font;
	GtkFontButton * button = GTK_FONT_BUTTON(fontButton);

	String txt = font.getName();
	txt += " ";
	txt += font.getSize();

	gtk_font_button_set_font_name(button, txt.c_str());
}

XojFont FontButton::getFont() {
	return font;
}

GtkToolItem * FontButton::createItem(bool horizontal) {
	if (item) {
		return item;
	}

	item = newItem();
	g_object_ref(item);
	gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(item), false);
	gtk_object_ref(GTK_OBJECT(item));
	g_signal_connect(fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);
	return item;
}

GtkToolItem * FontButton::newItem() {
	if (fontButton) {
		g_object_unref(fontButton);
	}
	GtkToolItem * it;

	it = gtk_tool_item_new();

	fontButton = gtk_font_button_new();
	gtk_widget_show(fontButton);
	gtk_container_add(GTK_CONTAINER(it), fontButton);
	gtk_font_button_set_use_font(GTK_FONT_BUTTON(fontButton), TRUE);
	gtk_button_set_focus_on_click(GTK_BUTTON(fontButton), FALSE);

	gtk_tool_item_set_tooltip_text(it, description.c_str());

	return it;
}

