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

class ToolbarItem {
public:
	ToolbarItem(String name);
	ToolbarItem();
	virtual ~ToolbarItem();

	operator String();

	bool operator == (ToolbarItem & other);

	int getId();

private:
	String name;
	int id;

	static int sid;
};

#endif /* __TOOLBARITEM_H__ */
