#include "ToolbarItem.h"

#include <utility>

int ToolbarItem::sid = 0;

ToolbarItem::ToolbarItem(string name) {
    this->name = std::move(name);
    this->id = ToolbarItem::sid++;

    if (ToolbarItem::sid < 0) {
        ToolbarItem::sid = 0;
    }
}

ToolbarItem::ToolbarItem() {
    this->name = "";
    this->id = -100;
}

auto ToolbarItem::getName() const -> const std::string& { return this->name; }

auto ToolbarItem::operator==(ToolbarItem& other) -> bool { return this->name == other.name; }

auto ToolbarItem::getId() const -> int { return this->id; }
