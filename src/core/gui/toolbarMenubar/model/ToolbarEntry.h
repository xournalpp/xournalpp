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

typedef std::vector<ToolbarItem*> ToolbarItemVector;

class ToolbarEntry {
public:
    ToolbarEntry();
    ToolbarEntry(const ToolbarEntry& e);
    ToolbarEntry(ToolbarEntry&& e);
    ~ToolbarEntry();

    ToolbarEntry& operator=(const ToolbarEntry& e);
    ToolbarEntry& operator=(ToolbarEntry&& e);

public:
    void clearList();

    std::string getName();
    void setName(std::string name);

    /**
     * Adds a new item and return the ID of the item
     */
    int addItem(std::string item);
    bool removeItemById(int id);

    /**
     * Insert a new item and return the ID of the item
     */
    int insertItem(std::string item, int position);

    const ToolbarItemVector& getItems() const;

private:
    std::string name;
    ToolbarItemVector entries;
};
