/*
 * Xournal++
 *
 * Undo action for page copy
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class CopyUndoAction : public UndoAction
{
public:
	CopyUndoAction(PageRef pageref, int pageNr);
	virtual ~CopyUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	int pageNr;
};
