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

#define ToolDrawCombocontrol_EntryCount 5

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

	virtual void actionPerformed(ActionType type, ActionGroup group,
	                             GdkEvent* event, GtkMenuItem* menuitem,
	                             GtkToolButton* toolbutton, bool enabled);

private:
	XOJ_TYPE_ATTRIB;

	ToolMenuHandler* toolMenuHandler;

	GtkWidget* iconWidget;
	GtkWidget* labelWidget;

	GdkPixbuf* icons[ToolDrawCombocontrol_EntryCount];
};
