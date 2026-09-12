#include "SpacerItem.h"

#include <gtk/gtk.h>

#include "util/i18n.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

static constexpr const char* ICON_NAME = "xopp-spacer";

SpacerItem::SpacerItem(const char* id): ItemWithNamedIcon(id, Category::SEPARATORS) {}

auto SpacerItem::createItem(ToolbarSide side) -> Widgetry {
    GtkWidget* it;
    if (to_Orientation(side) == GTK_ORIENTATION_HORIZONTAL) {
        it = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
        gtk_widget_set_hexpand(it, true);
    } else {
        it = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_widget_set_vexpand(it, true);
    }
    gtk_widget_add_css_class(it, "spacer");

    return {xoj::util::WidgetSPtr(it, xoj::util::adopt), nullptr};
}

auto SpacerItem::getToolDisplayName() const -> std::string { return _("Spacer"); }
auto SpacerItem::getIconName() const -> const char* { return ICON_NAME; }
