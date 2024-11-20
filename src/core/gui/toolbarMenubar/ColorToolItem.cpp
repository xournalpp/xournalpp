#include "ColorToolItem.h"

#include <utility>  // for move

#include "enums/Action.enum.h"                  // for Action
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon

ColorToolItem::ColorToolItem(NamedColor namedColor):
        AbstractToolItem(std::string("COLOR(") + std::to_string(namedColor.getIndex()) + ")", Category::COLORS),
        namedColor(std::move(namedColor)),
        target(xoj::util::makeGVariantSPtr(namedColor.getColor())) {}

ColorToolItem::~ColorToolItem() = default;

auto ColorToolItem::getColor() const -> Color { return this->namedColor.getColor(); }

auto ColorToolItem::createItem(ToolbarSide) -> Widgetry {
    auto* btn = gtk_toggle_button_new();
    auto actionName = std::string("win.") + Action_toString(Action::TOOL_COLOR);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());

    gtk_widget_set_tooltip_text(btn, this->namedColor.getName().c_str());
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());

    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [this]() {
        GtkWidget* proxy = gtk_check_button_new();
        gtk_widget_add_css_class(proxy, "model");

#if GTK_CHECK_VERSION(4, 8, 0)
        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_check_button_set_child(GTK_CHECK_BUTTON(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));
#else
        gtk_check_button_set_label(GTK_CHECK_BUTTON(proxy), getToolDisplayName().c_str());
#endif

        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(Action::TOOL_COLOR)).c_str());
        if (target) {
            gtk_actionable_set_action_target_value(GTK_ACTIONABLE(proxy), target.get());
        }
        return proxy;
    };
    return {xoj::util::WidgetSPtr(btn, xoj::util::adopt), xoj::util::WidgetSPtr(createProxy(), xoj::util::adopt)};
}

auto ColorToolItem::getToolDisplayName() const -> std::string { return this->namedColor.getName(); }

auto ColorToolItem::getNewToolIcon() const -> GtkWidget* {
    return ColorIcon::newGtkImage(this->namedColor.getColor(), true);
}

auto ColorToolItem::createPaintable(GdkSurface*) const -> xoj::util::GObjectSPtr<GdkPaintable> {
    return ColorIcon::newGdkPaintable(namedColor.getColor(), true);
}

void ColorToolItem::updateColor(const Palette& palette) { namedColor = palette.getColorAt(namedColor.getIndex()); }
