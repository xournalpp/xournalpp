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
#include <StringUtils.h>
#include <XournalType.h>

#include <gtk/gtk.h>

class AbstractItem : public ActionEnabledListener,
	public ActionSelectionListener
{
public:
	AbstractItem(string id, ActionHandler* handler, ActionType action,
				 GtkWidget* menuitem = NULL);
	virtual ~AbstractItem();

public:
	static void menuCallback(GtkMenuItem* menuitem, AbstractItem* toolItem);
	virtual void actionSelected(ActionGroup group, ActionType action);

	/**
	 * Override this method
	 */
	virtual void selected(ActionGroup group, ActionType action);

	virtual void actionEnabledAction(ActionType action, bool enabled);
	virtual void activated(GdkEvent* event, GtkMenuItem* menuitem,
						   GtkToolButton* toolbutton);

	virtual string getId();

	void setTmpDisabled(bool disabled);
	bool isEnabled();

protected:
	virtual void enable(bool enabled);

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
};
