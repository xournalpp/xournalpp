/*
 * Xournal++
 *
 * Abstract Toolbar / Menubar entry
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "control/Actions.h"

#include <XournalType.h>

#include <gtk/gtk.h>

class AbstractItem : public ActionEnabledListener, public ActionSelectionListener
{
public:
	AbstractItem(string id, ActionHandler* handler, ActionType action, GtkWidget* menuitem = NULL);
	virtual ~AbstractItem();

public:
	virtual void actionSelected(ActionGroup group, ActionType action);

	/**
	 * Override this method
	 */
	virtual void selected(ActionGroup group, ActionType action);

	virtual void actionEnabledAction(ActionType action, bool enabled);
	virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);

	virtual string getId();

	void setTmpDisabled(bool disabled);
	bool isEnabled();

	ActionType getActionType();

	/**
	 * Register a menu item. If there is already one registered, the new one will be ignored
	 */
	void setMenuItem(GtkWidget* menuitem);

protected:
	virtual void enable(bool enabled);

	virtual void actionPerformed(ActionType action, ActionGroup group,
								 GdkEvent* event, GtkMenuItem* menuitem,
								 GtkToolButton* toolbutton, bool selected);

private:
	XOJ_TYPE_ATTRIB;

protected:
	ActionGroup group;
	ActionType action;

	string id;

	ActionHandler* handler;

	bool enabled;

private:
	gulong menuSignalHandler;
	GtkWidget* menuitem;

	/**
	 * This is a check menu item which is not displayed as radio
	 */
	bool checkMenuItem;

	/**
	 * ignore event if the menu is programmatically changed
	 */
	bool ignoreNextCheckMenuEvent;

	/**
	 * Keep the state for toggle / radio menu handling
	 */
	bool itemActive;
};
