#include "AbstractToolItem.h"

#include <utility>  // for move

AbstractToolItem::AbstractToolItem(std::string id, Category cat): id(std::move(id)), category(cat) {}

AbstractToolItem::~AbstractToolItem() = default;

auto AbstractToolItem::getId() const -> const std::string& { return id; }
auto AbstractToolItem::getCategory() const -> Category { return category; }
