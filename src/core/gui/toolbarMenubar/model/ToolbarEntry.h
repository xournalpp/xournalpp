/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string
#include <vector>  // for vector

class ToolbarItem;

typedef std::vector<ToolbarItem> ToolbarItemVector;

/// Corresponds to to one ToolbarBox
class ToolbarEntry {
public:
    ToolbarEntry();
    ToolbarEntry(const ToolbarEntry& e);
    ToolbarEntry(ToolbarEntry&& e);
    ~ToolbarEntry();

    ToolbarEntry& operator=(const ToolbarEntry& e);
    ToolbarEntry& operator=(ToolbarEntry&& e);

public:
    std::string getName() const;
    void setName(std::string name);

    void addItem(std::string item);

    const ToolbarItemVector& getItems() const;

private:
    std::string name;
    ToolbarItemVector entries;
};
