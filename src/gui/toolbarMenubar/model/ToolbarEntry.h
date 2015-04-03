/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __TOOLBARENTRY_H__
#define __TOOLBARENTRY_H__

#include <vector>

#include "ToolbarItem.h"
typedef std::vector<ToolbarItem*> ToolbarItemVector;

class ToolbarEntry
{
public:
	ToolbarEntry();
	ToolbarEntry(const ToolbarEntry& e);
	~ToolbarEntry();

	void operator=(const ToolbarEntry& e);

	void clearList();

public:
	string getName();
	void setName(string name);

	/**
	 * Adds a new item and return the ID of the item
	 */
	int addItem(string item);
	bool removeItemById(int id);

	/**
	 * Insert a new item and return the ID of the item
	 */
	int insertItem(string item, int position);

	ToolbarItemVector* getItems();

private:
	XOJ_TYPE_ATTRIB;

	string name;
	ToolbarItemVector entries;
};

#endif /* __TOOLBARENTRY_H__ */
