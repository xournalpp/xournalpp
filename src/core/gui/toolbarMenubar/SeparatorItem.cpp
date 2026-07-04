#include "SeparatorItem.h"

#include <gtk/gtk.h>

#include "util/i18n.h"
#include "util/raii/GObjectSPtr.h"  // for WidgetSPtr

static constexpr const char* ICON_NAME = "xopp-separator";

SeparatorItem::SeparatorItem(const char* id): AbstractToolItem(id, Category::SEPARATORS) {}

auto SeparatorItem::createItem(bool horizontal) -> xoj::util::WidgetSPtr {
    return xoj::util::WidgetSPtr(GTK_WIDGET(gtk_separator_tool_item_new()), xoj::util::adopt);
}

auto SeparatorItem::getToolDisplayName() const -> std::string { return _("Separator"); }

auto SeparatorItem::getNewToolIcon() const -> GtkWidget* {
    return gtk_image_new_from_icon_name(ICON_NAME, GTK_ICON_SIZE_LARGE_TOOLBAR);
}
