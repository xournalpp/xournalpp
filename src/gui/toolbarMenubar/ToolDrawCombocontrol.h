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
class ToolDrawType;

class ToolDrawCombocontrol : public ToolButton
{
public:
	ToolDrawCombocontrol(ToolMenuHandler* toolMenuHandler, ActionHandler* handler, string id);
	virtual ~ToolDrawCombocontrol();

public:
	virtual void selected(ActionGroup group, ActionType action);

protected:
	virtual GtkToolItem* newItem();
	void createMenuItem(string name, string icon, ActionType type);

private:
	ToolMenuHandler* toolMenuHandler = nullptr;

	GtkWidget* iconWidget = nullptr;
	GtkWidget* labelWidget = nullptr;

	vector<ToolDrawType *> drawTypes;
};
