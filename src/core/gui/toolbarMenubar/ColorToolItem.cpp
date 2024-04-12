#include "ColorToolItem.h"

#include <utility>  // for move

#include "enums/Action.enum.h"                  // for Action
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon
#include "util/gtk4_helper.h"                   // for gtk_button_set_child

ColorToolItem::ColorToolItem(NamedColor namedColor, const std::optional<Recolor>& recolor):
        AbstractToolItem(std::string("COLOR(") + std::to_string(namedColor.getIndex()) + ")", Category::COLORS),
        namedColor(std::move(namedColor)),
        target(xoj::util::makeGVariantSPtr(namedColor.getColor())) {
    if (recolor) {
        secondaryColor = std::make_optional(recolor->convertColor(namedColor.getColor()));
    } else {
        secondaryColor = std::nullopt;
    }
}

ColorToolItem::~ColorToolItem() = default;

auto ColorToolItem::getColor() const -> Color { return this->namedColor.getColor(); }

auto ColorToolItem::createItem(bool) -> xoj::util::WidgetSPtr {
    auto* btn = gtk_toggle_button_new();
    auto actionName = std::string("win.") + Action_toString(Action::TOOL_COLOR);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());

    gtk_widget_set_tooltip_text(btn, this->namedColor.getName().c_str());
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());

    return xoj::util::WidgetSPtr(btn, xoj::util::adopt);
}

auto ColorToolItem::getToolDisplayName() const -> std::string { return this->namedColor.getName(); }

auto ColorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(this->namedColor.getColor(), 16, true, this->secondaryColor);
}

void ColorToolItem::updateColor(const Palette& palette) { namedColor = palette.getColorAt(namedColor.getIndex()); }

void ColorToolItem::updateSecondaryColor(const std::optional<Recolor>& recolor) {
    if (recolor) {
        secondaryColor = std::make_optional(recolor->convertColor(namedColor.getColor()));
    } else {
        secondaryColor = std::nullopt;
    }
}
