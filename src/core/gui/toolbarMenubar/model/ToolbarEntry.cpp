#include "ToolbarEntry.h"

#include <algorithm>
#include <utility>  // for move

#include <glib.h>

#include "gui/toolbarMenubar/model/ToolbarItem.h"  // for ToolbarItem
#include "util/Assert.h"

ToolbarEntry::ToolbarEntry() = default;

ToolbarEntry::ToolbarEntry(const ToolbarEntry& e) { *this = e; }
ToolbarEntry::ToolbarEntry(ToolbarEntry&& e) { *this = std::move(e); }

ToolbarEntry& ToolbarEntry::operator=(const ToolbarEntry& e) {
    this->name = e.name;
    this->entries = e.entries;
    return *this;
}

ToolbarEntry& ToolbarEntry::operator=(ToolbarEntry&& e) {
    this->name = std::move(e.name);
    this->entries = std::move(e.entries);
    return *this;
}

ToolbarEntry::~ToolbarEntry() = default;

auto ToolbarEntry::getName() const -> std::string { return this->name; }

void ToolbarEntry::setName(std::string name) { this->name = std::move(name); }

auto ToolbarEntry::addItem(std::string item) -> int { return entries.emplace_back(std::move(item)).getId(); }

auto ToolbarEntry::removeItemById(int id) -> bool {
    auto it = std::find_if(entries.begin(), entries.end(), [id](const auto& it) { return it.getId() == id; });
    if (it != entries.end()) {
        entries.erase(it);
        return true;
    }
    g_warning("ToolbarEntry::removeItemById: Tried to remove inexistant ID");
    return false;
}

auto ToolbarEntry::insertItem(std::string item, int position) -> int {
    xoj_assert(position >= 0);
    if (position >= static_cast<int>(entries.size())) {
        return entries.emplace_back(std::move(item)).getId();
    }

    return entries.emplace(std::next(entries.begin(), position), std::move(item))->getId();
}

auto ToolbarEntry::getItems() const -> const ToolbarItemVector& { return entries; }
