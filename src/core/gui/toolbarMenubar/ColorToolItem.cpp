#include "ColorToolItem.h"

#include <utility>  // for move

#include "enums/Action.enum.h"                  // for Action
#include "gui/toolbarMenubar/icon/ColorIcon.h"  // for ColorIcon
#include "util/GtkUtil.h"                       // for setToggleButtonUnreleasable
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
    gtk_widget_set_can_focus(btn, false);  // todo(gtk4) not necessary anymore
    auto actionName = std::string("win.") + Action_toString(Action::TOOL_COLOR);
    gtk_actionable_set_action_name(GTK_ACTIONABLE(btn), actionName.data());
    gtk_actionable_set_action_target_value(GTK_ACTIONABLE(btn), target.get());
    xoj::util::gtk::setToggleButtonUnreleasable(GTK_TOGGLE_BUTTON(btn));

    gtk_widget_set_tooltip_text(btn, this->namedColor.getName().c_str());
    gtk_button_set_child(GTK_BUTTON(btn), getNewToolIcon());


    // For the sake of deprecated GtkToolbar, wrap the button in a GtkToolItem
    // Todo(gtk4): remove
    GtkToolItem* it = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(it), btn);
    /// Makes a proxy item for the toolbar's overflow menu
    auto createProxy = [this]() {
        GtkWidget* proxy = gtk_check_menu_item_new();
        gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(proxy), true);

        auto* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
        gtk_container_add(GTK_CONTAINER(proxy), box);
        gtk_box_append(GTK_BOX(box), getNewToolIcon());
        gtk_box_append(GTK_BOX(box), gtk_label_new(getToolDisplayName().c_str()));

        gtk_actionable_set_action_name(GTK_ACTIONABLE(proxy),
                                       (std::string("win.") + Action_toString(Action::TOOL_COLOR)).c_str());
        if (target) {
            gtk_actionable_set_action_target_value(GTK_ACTIONABLE(proxy), target.get());
        }
        xoj::util::gtk::fixActionableInitialSensitivity(GTK_ACTIONABLE(proxy));
        return proxy;
    };
    gtk_tool_item_set_proxy_menu_item(it, "", createProxy());
    return xoj::util::WidgetSPtr(GTK_WIDGET(it), xoj::util::adopt);
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
