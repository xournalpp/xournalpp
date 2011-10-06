/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLBARENTRY_H__
#define __TOOLBARENTRY_H__

#include "ToolbarItem.h"
#include <gtk/gtk.h>
#include "../../../util/ListIterator.h"

class ToolbarEntry {
public:
	ToolbarEntry();
	ToolbarEntry(const ToolbarEntry & e);
	~ToolbarEntry();

	void operator = (const ToolbarEntry & e);

	void clearList();

public:
	String getName();
	void setName(String name);

	void addItem(String item);
	bool removeItemById(int id);
	void insertItem(String item, int position);

	ListIterator<ToolbarItem *> iterator();

private:
	XOJ_TYPE_ATTRIB;

	String name;
	GList * entries;
};

#endif /* __TOOLBARENTRY_H__ */
