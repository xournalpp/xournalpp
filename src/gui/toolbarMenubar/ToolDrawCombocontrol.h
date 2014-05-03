/*
 * Xournal++
 *
 * Part of the customizable toolbars
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __TOOLDRAWCOMBOCONTROL_H__
#define __TOOLDRAWCOMBOCONTROL_H__

#include "ToolButton.h"
#include "../GladeGui.h"
#include <XournalType.h>

class ToolMenuHandler;

class ToolDrawCombocontrol: public ToolButton
{
public:
	ToolDrawCombocontrol(ToolMenuHandler* th, ActionHandler* handler,
	                       GladeGui* gui, String id);
	virtual ~ToolDrawCombocontrol();

public:
	virtual void selected(ActionGroup group, ActionType action);

protected:
	virtual GtkToolItem* newItem();

private:
	XOJ_TYPE_ATTRIB;

	GtkWidget* iconWidget;
	GtkWidget* labelWidget;

	GdkPixbuf* iconDrawRect;
	GdkPixbuf* iconDrawCirc;
	GdkPixbuf* iconDrawArr;
};

#endif /* __TOOLDRAWCOMBOCONTROL_H__ */
