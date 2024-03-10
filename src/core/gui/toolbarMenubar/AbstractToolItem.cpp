#include "AbstractToolItem.h"

#include <utility>  // for move

AbstractToolItem::AbstractToolItem(std::string id, Category cat): id(std::move(id)), category(cat) {}

AbstractToolItem::~AbstractToolItem() = default;

auto AbstractToolItem::getId() const -> const std::string& { return id; }
auto AbstractToolItem::getCategory() const -> Category { return category; }

xoj::util::WidgetSPtr AbstractToolItem::createToolItem(bool horizontal) {
    xoj::util::WidgetSPtr item = createItem(horizontal);
    gtk_widget_set_can_focus(item.get(), false);  // todo(gtk4) not necessary anymore
    if (GTK_IS_TOOL_ITEM(item.get())) {
        gtk_widget_show_all(item.get());
        return item;
    }

    // Wrap in a GtkToolItem
    GtkToolItem* wrap = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(wrap), item.get());
    gtk_widget_show_all(GTK_WIDGET(wrap));
    return xoj::util::WidgetSPtr(GTK_WIDGET(wrap), xoj::util::adopt);
}
