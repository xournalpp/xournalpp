/*
 * Xournal++
 *
 * Handler for actions, every menu action, tool button etc. is defined here
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "enums/ActionType.enum.h"
#include "enums/ActionGroup.enum.h"

#include <XournalType.h>

#include <gtk/gtk.h>
#include <list>

class ActionHandler;

class ActionEnabledListener
{
public:
	ActionEnabledListener();
	virtual ~ActionEnabledListener();

public:
	virtual void actionEnabledAction(ActionType action, bool enabled) = 0;

	void registerListener(ActionHandler* handler);
	void unregisterListener();

private:
	ActionHandler* handler = nullptr;
};

class ActionSelectionListener
{
public:
	ActionSelectionListener();
	virtual ~ActionSelectionListener();

	virtual void actionSelected(ActionGroup group, ActionType action) = 0;

	void registerListener(ActionHandler* handler);
	void unregisterListener();

private:
	ActionHandler* handler;
};

class ActionHandler
{
public:
	ActionHandler();
	virtual ~ActionHandler();

public:
	virtual void actionPerformed(ActionType type, ActionGroup group,
	                             GdkEvent* event, GtkMenuItem* menuitem,
	                             GtkToolButton* toolbutton, bool enabled) = 0;

	void fireEnableAction(ActionType action, bool enabled);
	void addListener(ActionEnabledListener* listener);
	void removeListener(ActionEnabledListener* listener);

	void fireActionSelected(ActionGroup group, ActionType action);
	void addListener(ActionSelectionListener* listener);
	void removeListener(ActionSelectionListener* listener);

private:
	std::list<ActionEnabledListener*> enabledListener;
	std::list<ActionSelectionListener*> selectionListener;
};
