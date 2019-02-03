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
#include "control/ToolHandler.h"

#include <XournalType.h>

class ColorToolItem : public AbstractToolItem
{
public:
	ColorToolItem(ActionHandler* handler, ToolHandler* toolHandler, GtkWindow* parent, int color,
			bool selektor = false);
	virtual ~ColorToolItem();

public:
	virtual void actionSelected(ActionGroup group, ActionType action);
	void enableColor(int color);
	bool colorEqualsMoreOreLess(int color);
	virtual void activated(GdkEvent* event, GtkMenuItem* menuitem, GtkToolButton* toolbutton);

	virtual string getToolDisplayName();
	virtual GtkWidget* getNewToolIcon();

	virtual string getId();

	int getColor();

protected:
	virtual GtkToolItem* newItem();
	void updateName();
	bool isSelector();

	/**
	 * Show colochooser to select a custom color
	 */
	void showColorchooser();

private:
	XOJ_TYPE_ATTRIB;

	int color;
	string name;
	GtkWidget* iconWidget = NULL;
	GtkWindow* parent;

	ToolHandler* toolHandler;

	static bool inUpdate;
};
