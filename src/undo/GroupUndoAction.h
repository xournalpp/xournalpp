/*
 * Xournal++
 *
 * Undo action to group a list of undo actions of the same type
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class GroupUndoAction : public UndoAction
{
public:
	GroupUndoAction();
	virtual ~GroupUndoAction();

public:
	void addAction(UndoAction* action);

	/**
	 * Get the affected pages
	 */
	virtual vector<PageRef> getPages();

	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	vector<UndoAction*> actions;
};
