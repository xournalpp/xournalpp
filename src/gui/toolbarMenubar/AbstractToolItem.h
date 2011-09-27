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

#ifndef __ABSTRACTTOOLITEM_H__
#define __ABSTRACTTOOLITEM_H__

#include "AbstractItem.h"

class AbstractToolItem: public AbstractItem {
public:
	AbstractToolItem(String id, ActionHandler * handler, ActionType type, GtkWidget * menuitem = NULL);
	virtual ~AbstractToolItem();

public:
	virtual void selected(ActionGroup group, ActionType action);
	virtual GtkToolItem * createItem(bool horizontal);
	virtual GtkToolItem * createTmpItem(bool horizontal);
	void setPopupMenu(GtkWidget * popupMenu);
	GtkWidget * getPopupMenu();

	bool isUsed();
	void setUsed(bool used);

	bool containsWidget(GtkWidget * widget);

	static void toolButtonCallback(GtkToolButton *toolbutton, AbstractToolItem * item);

	virtual String getToolDisplayName() = 0;
	virtual GtkWidget * getNewToolIcon() = 0;

protected:
	virtual GtkToolItem * newItem() = 0;

	virtual void enable(bool enabled);

public:
	XOJ_TYPE_ATTRIB;

protected:
	GtkToolItem * item;
	GtkWidget * popupMenu;

	bool toolToggleButtonActive;
	bool toolToggleOnlyEnable;

	bool used;
};

#endif /* __ABSTRACTTOOLITEM_H__ */
