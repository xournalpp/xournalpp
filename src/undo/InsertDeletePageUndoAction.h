/*
 * Xournal++
 *
 * Undo action for insert page / delete page
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class InsertDeletePageUndoAction : public UndoAction
{
public:
	InsertDeletePageUndoAction(PageRef page, int pagePos, bool inserted);
	virtual ~InsertDeletePageUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();
private:
	bool insertPage(Control* control);
	bool deletePage(Control* control);

private:
	bool inserted;
	int pagePos;
};
