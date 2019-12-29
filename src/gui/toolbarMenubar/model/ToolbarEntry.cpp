#include "ToolbarEntry.h"

#include <utility>

ToolbarEntry::ToolbarEntry() = default;

ToolbarEntry::ToolbarEntry(const ToolbarEntry& e) { *this = e; }

void ToolbarEntry::operator=(const ToolbarEntry& e) {
    this->name = e.name;
    clearList();

    for (ToolbarItem* item: e.entries) {
        entries.push_back(new ToolbarItem(*item));
    }
}

ToolbarEntry::~ToolbarEntry() { clearList(); }

void ToolbarEntry::clearList() {
    for (ToolbarItem* item: entries) {
        delete item;
    }
    entries.clear();
}

auto ToolbarEntry::getName() -> string { return this->name; }

void ToolbarEntry::setName(string name) { this->name = std::move(name); }

auto ToolbarEntry::addItem(string item) -> int {
    auto* it = new ToolbarItem(std::move(item));
    entries.push_back(it);

    return it->getId();
}

auto ToolbarEntry::removeItemById(int id) -> bool {
    for (unsigned int i = 0; i < this->entries.size(); i++) {
        if (this->entries[i]->getId() == id) {
            delete this->entries[i];
            entries[i] = nullptr;
            entries.erase(entries.begin() + i);
            return true;
        }
    }
    return false;
}

auto ToolbarEntry::insertItem(string item, int position) -> int {
    auto* it = new ToolbarItem(std::move(item));
    if (position >= static_cast<int>(entries.size())) {
        entries.push_back(it);
        return it->getId();
    }

    entries.insert(entries.begin() + position, it);
    return it->getId();
}

auto ToolbarEntry::getItems() const -> const ToolbarItemVector& { return entries; }
