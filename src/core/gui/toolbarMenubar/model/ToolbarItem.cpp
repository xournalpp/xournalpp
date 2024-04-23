#include "ToolbarItem.h"

#include <utility>

ToolbarItem::ToolbarItem(std::string name): name(std::move(name)) {}

auto ToolbarItem::getName() const -> std::string { return this->name; }

void ToolbarItem::setName(std::string name) { this->name = std::move(name); }

auto ToolbarItem::operator==(ToolbarItem& other) -> bool { return this->name == other.name; }
