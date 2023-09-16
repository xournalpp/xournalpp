#include "ColorToolItem.h"

#include <array>    // for array
#include <cstdio>   // for snprintf, size_t
#include <memory>   // for unique_ptr
#include <sstream>  // for ostringstream
#include <utility>  // for move

#include <glib.h>  // for gchar

#include "control/actions/ActionDatabase.h"
#include "gui/toolbarMenubar/AbstractToolItem.h"       // for AbstractToolItem
#include "gui/toolbarMenubar/icon/ColorIcon.h"         // for ColorIcon
#include "util/GtkUtil.h"                              // for setToggleButtonUnreleasable
#include "util/gtk4_helper.h"                          // for gtk_button_set_child
#include "util/i18n.h"                                 // for _

ColorToolItem::ColorToolItem(NamedColor namedColor):
        AbstractToolItem(std::string("COLOR(") + std::to_string(namedColor.getIndex()) + ")"),
        namedColor(std::move(namedColor)),
        target(xoj::util::makeGVariantSPtr(static_cast<uint32_t>(namedColor.getColor()))) {}

ColorToolItem::~ColorToolItem() = default;

auto ColorToolItem::getColor() const -> Color { return this->namedColor.getColor(); }

auto ColorToolItem::createItem(bool) -> GtkWidget* {
    auto* btn = gtk_toggle_button_new();
    auto actionName = std::string("win.") + Action_toString(Action::TOOL_COLOR);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));

    gtk_widget_set_tooltip_text(btn, this->namedColor.getName().c_str());
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());

    this->item.reset(GTK_WIDGET(btn), xoj::util::adopt);
    return this->item.get();
}

auto ColorToolItem::getToolDisplayName() const -> std::string { return this->namedColor.getName(); }

auto ColorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(this->namedColor.getColor(), 16, true);
}
