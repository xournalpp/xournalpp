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

class ColorToolItem: public AbstractToolItem {
public:
	ColorToolItem(String id, ActionHandler * handler, ToolHandler * toolHandler, int color, bool selector = false);
	virtual ~ColorToolItem();

public:
	virtual void actionSelected(ActionGroup group, ActionType action);
	void enableColor(int color);
	void selectColor();
	bool colorEqualsMoreOreLess(int color);
	virtual void activated(GdkEvent * event, GtkMenuItem * menuitem, GtkToolButton * toolbutton);

protected:
	virtual GtkToolItem * newItem();
	static void customColorSelected(GtkWidget * button, ColorToolItem * item);

private:
	int color;
	bool selector;
	GtkWidget * iconWidget;
	GtkWidget * colorDlg;

	ToolHandler * toolHandler;

	static bool inUpdate;
};

#endif /* __COLORTOOLITEM_H__ */
