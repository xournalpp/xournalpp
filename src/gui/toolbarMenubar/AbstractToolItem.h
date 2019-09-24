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
	AbstractToolItem(string id, ActionHandler* handler, ActionType type, GtkWidget* menuitem = nullptr);
	virtual ~AbstractToolItem();

public:
	virtual void selected(ActionGroup group, ActionType action);
	virtual GtkToolItem* createItem(bool horizontal);
	virtual GtkToolItem* createTmpItem(bool horizontal);
	void setPopupMenu(GtkWidget* popupMenu);

	bool isUsed();
	void setUsed(bool used);

	static void toolButtonCallback(GtkToolButton* toolbutton, AbstractToolItem* item);

	virtual string getToolDisplayName() = 0;
	virtual GtkWidget* getNewToolIcon() = 0;

	/**
	 * Enable / Disable the tool item
	 */
	virtual void enable(bool enabled);

protected:
	virtual GtkToolItem* newItem() = 0;

public:
	protected:
	GtkToolItem* item = nullptr;
	GtkWidget* popupMenu = nullptr;

	bool toolToggleButtonActive = false;
	bool toolToggleOnlyEnable = false;

	/**
	 * This item is already somewhere in the toolbar
	 */
	bool used = false;
};
