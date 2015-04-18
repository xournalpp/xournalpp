/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "AbstractToolItem.h"
#include "../GladeGui.h"

class ToolButton : public AbstractToolItem
{
public:
	ToolButton(ActionHandler* handler, string id, ActionType type, string stock,
			   string description, GtkWidget* menuitem = NULL);
	ToolButton(ActionHandler* handler, GladeGui* gui, string id, ActionType type,
			   string iconName, string description, GtkWidget* menuitem = NULL);
	ToolButton(ActionHandler* handler, GladeGui* gui, string id, ActionType type,
			   ActionGroup group, bool toolToggleOnlyEnable, string iconName,
			   string description,
			   GtkWidget* menuitem = NULL);

	virtual ~ToolButton();

public:
	void updateDescription(string description);
	virtual string getToolDisplayName();

protected:
	virtual GtkToolItem* newItem();

	virtual GtkWidget* getNewToolIconImpl();

protected:
	GladeGui* gui;

private:
	XOJ_TYPE_ATTRIB;

	string stock;

	string iconName;
	string description;
};
