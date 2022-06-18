#include "ToolbarEntry.h"

#include <utility>  // for move

#include "gui/toolbarMenubar/model/ToolbarItem.h"  // for ToolbarItem

using std::string;

ToolbarEntry::ToolbarEntry() = default;

ToolbarEntry::ToolbarEntry(const ToolbarEntry& e) { *this = e; }
ToolbarEntry::ToolbarEntry(ToolbarEntry&& e) { *this = std::move(e); }

ToolbarEntry& ToolbarEntry::operator=(const ToolbarEntry& e) {
    this->name = e.name;
    std::vector<ToolbarItem*> entries;
    for (ToolbarItem* item: e.entries) { entries.push_back(new ToolbarItem(*item)); }
    clearList();
    this->entries = std::move(entries);
    return *this;
}

ToolbarEntry& ToolbarEntry::operator=(ToolbarEntry&& e) {
    this->name = std::move(e.name);
    std::swap(this->entries, e.entries);
    return *this;
}

ToolbarEntry::~ToolbarEntry() { clearList(); }

void ToolbarEntry::clearList() {
    for (ToolbarItem* item: entries) { delete item; }
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
    for (auto it = this->entries.begin(); it != this->entries.end(); it++) {
        if ((*it)->getId() == id) {
            delete *it;
            this->entries.erase(it);
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
