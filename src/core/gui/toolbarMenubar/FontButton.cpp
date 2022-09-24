#include "FontButton.h"

#include <sstream>  // for stringstream, basic...
#include <utility>  // for move

#include <glib-object.h>  // for g_object_ref, g_obj...

#include "control/Actions.h"                      // for ActionHandler
#include "enums/ActionGroup.enum.h"               // for GROUP_NOGROUP
#include "gui/toolbarMenubar/AbstractToolItem.h"  // for AbstractToolItem
#include "util/i18n.h"                            // for _
#include "util/serdesstream.h"                    // for serdes_stream

class GladeGui;

using std::string;

FontButton::FontButton(ActionHandler* handler, GladeGui* gui, string id, ActionType type, string description,
                       GtkWidget* menuitem):
        AbstractToolItem(std::move(id), handler, type, menuitem) {
    this->gui = gui;
    this->description = std::move(description);
}

FontButton::~FontButton() = default;

void FontButton::activated(GtkMenuItem* menuitem, GtkToolButton* toolbutton) {
    GtkFontButton* button = GTK_FONT_BUTTON(fontButton);

    string name = gtk_font_button_get_font_name(button);

    auto pos = name.find_last_of(' ');
    this->font.setName(name.substr(0, pos));
    this->font.setSize(std::stod(name.substr(pos + 1)));

    handler->actionPerformed(ACTION_FONT_BUTTON_CHANGED, GROUP_NOGROUP, nullptr, true);
}

void FontButton::setFontFontButton(GtkWidget* fontButton, const XojFont& font) {
    GtkFontButton* button = GTK_FONT_BUTTON(fontButton);
    auto fontSizeStream = serdes_stream<std::stringstream>();
    fontSizeStream << font.getSize();
    string name = font.getName() + " " + fontSizeStream.str();
    gtk_font_button_set_font_name(button, name.c_str());
}

void FontButton::setFont(const XojFont& font) {
    this->font = font;
    if (this->fontButton == nullptr) {
        return;
    }

    setFontFontButton(this->fontButton, font);
}

auto FontButton::getFont() const -> XojFont {
    // essentially, copy the font object to prevent a memory leak.
    XojFont newfont;
    newfont.setName(font.getName());
    newfont.setSize(font.getSize());

    return newfont;
}

auto FontButton::getToolDisplayName() const -> string { return _("Font"); }

auto FontButton::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name("font-x-generic", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

auto FontButton::getNewToolPixbuf() const -> GdkPixbuf* { return getPixbufFromImageIconName(); }

auto FontButton::createItem(bool horizontal) -> GtkToolItem* {
    if (this->item) {
        return this->item;
    }

    this->item = newItem();
    g_object_ref(this->item);
    g_signal_connect(fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);
    return this->item;
}

auto FontButton::createTmpItem(bool horizontal) -> GtkToolItem* {
    GtkWidget* fontButton = newFontButton();

    GtkToolItem* it = gtk_tool_item_new();

    gtk_container_add(GTK_CONTAINER(it), fontButton);
    gtk_tool_item_set_tooltip_text(it, this->description.c_str());
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(it), false);

    if (!this->font.getName().empty()) {
        setFontFontButton(fontButton, this->font);
    }

    gtk_widget_show_all(GTK_WIDGET(it));
    return it;
}

void FontButton::showFontDialog() {
    if (this->fontButton == nullptr) {
        newItem();
    }

    gtk_button_clicked(GTK_BUTTON(this->fontButton));
}

auto FontButton::newFontButton() -> GtkWidget* {
    GtkWidget* w = gtk_font_button_new();
    gtk_widget_show(w);
    gtk_font_button_set_use_font(GTK_FONT_BUTTON(w), true);
    gtk_button_set_focus_on_click(GTK_BUTTON(w), false);

    return w;
}

auto FontButton::newItem() -> GtkToolItem* {
    if (this->fontButton) {
        g_object_unref(this->fontButton);
    }
    GtkToolItem* it = gtk_tool_item_new();

    this->fontButton = newFontButton();
    gtk_container_add(GTK_CONTAINER(it), this->fontButton);
    gtk_tool_item_set_tooltip_text(it, this->description.c_str());
    gtk_tool_item_set_homogeneous(GTK_TOOL_ITEM(it), false);

    g_signal_connect(this->fontButton, "font_set", G_CALLBACK(&toolButtonCallback), this);

    if (!this->font.getName().empty()) {
        setFont(this->font);
    }

    return it;
}
