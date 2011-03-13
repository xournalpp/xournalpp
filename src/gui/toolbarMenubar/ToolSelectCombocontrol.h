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

#ifndef __TOOLSELECTCOMBOCONTROL_H__
#define __TOOLSELECTCOMBOCONTROL_H__

#include "ToolButton.h"
#include "../GladeGui.h"

class ToolMenuHandler;

class ToolSelectCombocontrol: public ToolButton {
public:
	ToolSelectCombocontrol(ToolMenuHandler * th, ActionHandler * handler, GladeGui * gui, String id);
	virtual ~ToolSelectCombocontrol();

public:
	virtual void selected(ActionGroup group, ActionType action);

protected:
	virtual GtkToolItem * newItem();

private:
	GtkWidget * iconWidget;
	GtkWidget * labelWidget;

	GdkPixbuf * iconSelectRect;
	GdkPixbuf * iconSelectRgion;
	GdkPixbuf * iconSelectObject;
};

#endif /* __TOOLSELECTCOMBOCONTROL_H__ */
