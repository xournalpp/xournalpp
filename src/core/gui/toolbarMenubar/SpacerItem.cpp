#include "SpacerItem.h"

#include <gtk/gtk.h>

#include "util/i18n.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

static constexpr const char* ICON_NAME = "xopp-spacer";

SpacerItem::SpacerItem(const char* id): AbstractToolItem(id, Category::SEPARATORS) {}

auto SpacerItem::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    GtkToolItem* toolItem = gtk_separator_tool_item_new();
    gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(toolItem), false);
    gtk_tool_item_set_expand(toolItem, true);
    return xoj::util::WidgetSPtr(GTK_WIDGET(toolItem), xoj::util::adopt);
}

auto SpacerItem::getToolDisplayName() const -> std::string { return _("Spacer"); }

auto SpacerItem::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(ICON_NAME, GTK_ICON_SIZE_LARGE_TOOLBAR);
}
