/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "ToolButton.h"
#include "../GladeGui.h"
#include <XournalType.h>

class ToolMenuHandler;

class ToolSelectCombocontrol : public ToolButton
{
public:
	ToolSelectCombocontrol(ToolMenuHandler* th, ActionHandler* handler,
						   GladeGui* gui, string id);
	virtual ~ToolSelectCombocontrol();

public:
	virtual void selected(ActionGroup group, ActionType action);

protected:
	virtual GtkToolItem* newItem();

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* iconWidget;
	GtkWidget* labelWidget;

	GdkPixbuf* iconSelectRect;
	GdkPixbuf* iconSelectRgion;
	GdkPixbuf* iconSelectObject;
};
