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

#ifndef __MENUITEM_H__
#define __MENUITEM_H__

#include "AbstractItem.h"

/**
 * Menuitem handler
 */
class MenuItem : public AbstractItem
{
public:
	MenuItem(ActionHandler* handler, GtkWidget* widget, ActionType type);
	MenuItem(ActionHandler* handler, GtkWidget* widget, ActionType type,
			ActionGroup group);
	virtual ~MenuItem();

private:
	XOJ_TYPE_ATTRIB;
};

#endif /* __MENUITEM_H__ */
