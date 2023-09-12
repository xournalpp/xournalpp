#include "ColorToolItem.h"

#include <utility>  // for move

#include "enums/Action.enum.h"                  // for Action
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon
#include "util/GtkUtil.h"                       // for setToggleButtonUnreleasable
#include "util/gtk4_helper.h"                   // for gtk_button_set_child

ColorToolItem::ColorToolItem(NamedColor namedColor):
        AbstractToolItem(std::string("COLOR(") + std::to_string(namedColor.getIndex()) + ")", Category::COLORS),
        namedColor(std::move(namedColor)),
        target(xoj::util::makeGVariantSPtr(namedColor.getColor())) {}

ColorToolItem::~ColorToolItem() = default;

auto ColorToolItem::getColor() const -> Color { return this->namedColor.getColor(); }

auto ColorToolItem::createItem(bool) -> xoj::util::WidgetSPtr {
    auto* btn = gtk_toggle_button_new();
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    auto actionName = std::string("win.") + Action_toString(Action::TOOL_COLOR);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));

    gtk_widget_set_tooltip_text(btn, this->namedColor.getName().c_str());
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());

    return xoj::util::WidgetSPtr(btn, xoj::util::adopt);
}

auto ColorToolItem::getToolDisplayName() const -> std::string { return this->namedColor.getName(); }

auto ColorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(this->namedColor.getColor(), 16, true);
}
