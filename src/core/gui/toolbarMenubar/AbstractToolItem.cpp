#include "AbstractToolItem.h"

#include <utility>  // for move

AbstractToolItem::AbstractToolItem(std::string id): id(std::move(id)) {}

AbstractToolItem::~AbstractToolItem() = default;

auto AbstractToolItem::getId() const -> const std::string& { return id; }

auto AbstractToolItem::getItem() const -> GtkWidget* { return this->item.get(); }

GtkToolItem* AbstractToolItem::createToolItem(bool horizontal) {
    GtkWidget* item = createItem(horizontal);
    gtk_widget_set_can_focus(item, false);  // todo(gtk4) not necessary anymore
    if (GTK_IS_TOOL_ITEM(item)) {
        return GTK_TOOL_ITEM(item);
    }

    // Wrap in a GtkToolItem
    GtkToolItem* wrap = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(wrap), item);
    this->item.reset(GTK_WIDGET(wrap), xoj::util::adopt);
    return wrap;
}

auto AbstractToolItem::isUsed() const -> bool { return used; }

void AbstractToolItem::setUsed(bool used) { this->used = used; }
