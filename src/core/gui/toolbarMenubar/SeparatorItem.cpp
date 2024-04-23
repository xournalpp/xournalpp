#include "SeparatorItem.h"

#include <gtk/gtk.h>

#include "util/i18n.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

static constexpr const char* ICON_NAME = "xopp-separator";

SeparatorItem::SeparatorItem(const char* id): ItemWithNamedIcon(id, Category::SEPARATORS) {}

auto SeparatorItem::createItem(ToolbarSide side) -> Widgetry {
    auto* it = gtk_separator_new(to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL ? GTK_ORIENTATION_VERTICAL :
                                                                                      GTK_ORIENTATION_HORIZONTAL);
    auto* proxy = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_add_css_class(proxy, "model");
    return {xoj::util::WidgetSPtr(it, xoj::util::adopt), xoj::util::WidgetSPtr(proxy, xoj::util::adopt)};
}

auto SeparatorItem::getToolDisplayName() const -> std::string { return _("Separator"); }
auto SeparatorItem::getIconName() const -> const char* { return ICON_NAME; }
