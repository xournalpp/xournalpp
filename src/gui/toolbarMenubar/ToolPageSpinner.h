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

class ToolPageSpinner: public AbstractToolItem {
public:
	ToolPageSpinner(ActionHandler * handler, String id, ActionType type);
	virtual ~ToolPageSpinner();

public:
	GtkWidget * getPageSpinner();
	void setText(String text);

protected:
	virtual GtkToolItem * newItem();

private:
	GtkWidget * pageSpinner;
	GtkWidget * lbPageNo;
};

#endif /* __TOOLPAGESPINNER_H__ */
