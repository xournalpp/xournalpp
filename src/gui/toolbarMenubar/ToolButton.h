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

#include "AbstractToolItem.h"
#include "gui/GladeGui.h"

class ToolButton : public AbstractToolItem
{
public:
	ToolButton(ActionHandler* handler, string id, ActionType type, string iconName, string description,
			   GtkWidget* menuitem = nullptr);
	ToolButton(ActionHandler* handler, string id, ActionType type, ActionGroup group,
			   bool toolToggleOnlyEnable, string iconName, string description, GtkWidget* menuitem = nullptr);

	virtual ~ToolButton();

public:
	/**
	 * Register a popup menu entry, create a popup menu, if none is there
	 *
	 * @param name The name of the item
	 * @param iconName To load an icon
	 * @return The created menu item
	 */
	GtkWidget* registerPopupMenuEntry(string name, string iconName = "");

	void updateDescription(string description);
	virtual string getToolDisplayName();
	void setActive(bool active);

protected:
	virtual GtkToolItem* newItem();

	virtual GtkWidget* getNewToolIcon();

private:
	string iconName;
	string description;
};
