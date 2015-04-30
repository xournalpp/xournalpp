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

#include "AbstractItem.h"

class AbstractToolItem : public AbstractItem
{
public:
	AbstractToolItem(string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem = NULL);
	virtual ~AbstractToolItem();

public:
	virtual void selected(ActionGroup group, ActionType action);
	virtual GtkToolItem* createItem(bool horizontal);
	virtual GtkToolItem* createTmpItem(bool horizontal);
	void setPopupMenu(GtkWidget* popupMenu);
	GtkWidget* getPopupMenu();

	bool isUsed();
	void setUsed(bool used);

	static void toolButtonCallback(GtkToolButton* toolbutton, AbstractToolItem* item);

	virtual string getToolDisplayName() = 0;
	virtual GtkWidget* getNewToolIcon();

protected:
	virtual GtkToolItem* newItem() = 0;

	virtual void enable(bool enabled);

	virtual GtkWidget* getNewToolIconImpl() = 0;

public:
	XOJ_TYPE_ATTRIB;

protected:
	GtkToolItem* item;
	GtkWidget* popupMenu;

	bool toolToggleButtonActive;
	bool toolToggleOnlyEnable;

	bool used;
};
