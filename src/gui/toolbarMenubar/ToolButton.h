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

#ifndef __TOOLBUTTON_H__
#define __TOOLBUTTON_H__

#include "AbstractToolItem.h"
#include "../GladeGui.h"

class ToolButton: public AbstractToolItem
{
public:
	ToolButton(ActionHandler* handler, String id, ActionType type, String stock,
	           String description, GtkWidget* menuitem = NULL);
	ToolButton(ActionHandler* handler, GladeGui* gui, String id, ActionType type,
	           String iconName, String description, GtkWidget* menuitem = NULL);
	ToolButton(ActionHandler* handler, GladeGui* gui, String id, ActionType type,
	           ActionGroup group, bool toolToggleOnlyEnable, String iconName,
	           String description,
	           GtkWidget* menuitem = NULL);

	virtual ~ToolButton();

public:
	void updateDescription(String description);
	virtual String getToolDisplayName();

protected:
	virtual GtkToolItem* newItem();

	virtual GtkWidget* getNewToolIconImpl();

protected:
	GladeGui* gui;

private:
	XOJ_TYPE_ATTRIB;

	String stock;

	String iconName;
	String description;
};
#endif /* __TOOLBUTTON_H__ */
