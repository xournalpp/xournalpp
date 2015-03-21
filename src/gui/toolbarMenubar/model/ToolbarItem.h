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

#include <StringUtils.h>
#include <XournalType.h>

class ToolbarItem
{
public:
	ToolbarItem(string name);
	ToolbarItem(const ToolbarItem& item);
	ToolbarItem();
	virtual ~ToolbarItem();

	operator string();

	bool operator == (ToolbarItem& other);

	int getId();

private:
	XOJ_TYPE_ATTRIB;

	string name;
	int id;

	static int sid;
};

#endif /* __TOOLBARITEM_H__ */
