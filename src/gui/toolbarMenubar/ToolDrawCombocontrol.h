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

#include "ToolButton.h"
#include "gui/GladeGui.h"

#include <XournalType.h>

class ToolMenuHandler;

class ToolDrawCombocontrol : public ToolButton
{
public:
	ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, GladeGui* gui, string id);
	virtual ~ToolDrawCombocontrol();

public:
	virtual void selected(ActionGroup group, ActionType action);

protected:
	virtual GtkToolItem* newItem();
	void createMenuItem(string name, string icon, ActionType type);

private:
	XOJ_TYPE_ATTRIB;

	ToolMenuHandler* toolMenuHandler;

	GtkWidget* iconWidget;
	GtkWidget* labelWidget;

	GdkPixbuf* iconDrawRect;
	GdkPixbuf* iconDrawCirc;
	GdkPixbuf* iconDrawArr;
	GdkPixbuf* iconDrawLine;
	GdkPixbuf* iconAutoDrawLine;
};
