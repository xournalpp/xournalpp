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

#ifndef __COLORTOOLITEM_H__
#define __COLORTOOLITEM_H__

#include "AbstractToolItem.h"
#include "../../control/ToolHandler.h"
#include <XournalType.h>

class ColorToolItem: public AbstractToolItem {
public:
	ColorToolItem(ActionHandler * handler, ToolHandler * toolHandler, int color, bool selektor = false);
	virtual ~ColorToolItem();

public:
	virtual void actionSelected(ActionGroup group, ActionType action);
	void enableColor(int color);
	void selectColor();
	bool colorEqualsMoreOreLess(int color);
	virtual void activated(GdkEvent * event, GtkMenuItem * menuitem, GtkToolButton * toolbutton);

	virtual String getToolDisplayName();
	virtual GtkWidget * getNewToolIconImpl();

	virtual String getId();

	int getColor();

protected:
	virtual GtkToolItem * newItem();
	static void customColorSelected(GtkWidget * button, ColorToolItem * item);
	void updateName();
	bool isSelector();

private:
	XOJ_TYPE_ATTRIB;

	int color;
	String name;
	GtkWidget * iconWidget;
	GtkWidget * colorDlg;

	ToolHandler * toolHandler;

	static bool inUpdate;
};

#endif /* __COLORTOOLITEM_H__ */
