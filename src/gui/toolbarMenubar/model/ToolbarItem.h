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

#ifndef __TOOLBARITEM_H__
#define __TOOLBARITEM_H__

#include <String.h>
#include <XournalType.h>

class ToolbarItem
{
public:
	ToolbarItem(String name);
	ToolbarItem(const ToolbarItem& item);
	ToolbarItem();
	virtual ~ToolbarItem();

	operator String();

	bool operator == (ToolbarItem& other);

	int getId();

private:
	XOJ_TYPE_ATTRIB;

	String name;
	int id;

	static int sid;
};

#endif /* __TOOLBARITEM_H__ */
