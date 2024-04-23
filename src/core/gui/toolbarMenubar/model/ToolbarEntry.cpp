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

void ToolbarEntry::addItem(std::string item) { entries.emplace_back(std::move(item)); }

auto ToolbarEntry::getItems() const -> const ToolbarItemVector& { return entries; }
