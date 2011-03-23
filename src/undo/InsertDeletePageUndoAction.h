/*
 * Xournal++
 *
 * Undo action for insert page / delete page
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __INSERTDELETEPAGEUNDOACTION_H__
#define __INSERTDELETEPAGEUNDOACTION_H__

#include "UndoAction.h"
#include "../util/XournalType.h"

class InsertDeletePageUndoAction: public UndoAction {
public:
	InsertDeletePageUndoAction(XojPage * page, int pagePos, bool inserted);
	~InsertDeletePageUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	bool insertPage(Control * control);
	bool deletePage(Control * control);

private:
	XOJ_TYPE_ATTRIB;

	bool inserted;
	int pagePos;
};

#endif /* __INSERTDELETEPAGEUNDOACTION_H__ */
