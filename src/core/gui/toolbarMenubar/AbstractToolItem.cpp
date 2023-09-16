#include "AbstractToolItem.h"

#include <utility>  // for move

AbstractToolItem::AbstractToolItem(std::string id): id(std::move(id)) {}

AbstractToolItem::~AbstractToolItem() = default;

auto AbstractToolItem::getId() const -> std::string { return id; }

auto AbstractToolItem::getItem() const -> GtkWidget* { return this->item.get(); }

auto AbstractToolItem::isUsed() const -> bool { return used; }

void AbstractToolItem::setUsed(bool used) { this->used = used; }
