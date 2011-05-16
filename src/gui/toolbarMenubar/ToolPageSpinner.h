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

#ifndef __TOOLPAGESPINNER_H__
#define __TOOLPAGESPINNER_H__

#include "AbstractToolItem.h"
#include <XournalType.h>

class GladeGui;

class ToolPageSpinner: public AbstractToolItem {
public:
	ToolPageSpinner(GladeGui * gui, ActionHandler * handler, String id, ActionType type);
	virtual ~ToolPageSpinner();

public:
	GtkWidget * getPageSpinner();
	void setText(String text);
	virtual String getToolDisplayName();
	virtual GtkWidget * getNewToolIcon();

protected:
	virtual GtkToolItem * newItem();

private:
	XOJ_TYPE_ATTRIB;

	GladeGui * gui;

	GtkWidget * pageSpinner;
	GtkWidget * lbPageNo;
};

#endif /* __TOOLPAGESPINNER_H__ */
