/*
 * Xournal++
 *
 * Undo page swap action
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include "model/PageRef.h"

class Control;

class SwapUndoAction : public UndoAction
{
public:
	SwapUndoAction(size_t pageNr, bool moveUp, PageRef swappedPage, PageRef otherPage);

	virtual ~SwapUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	vector<PageRef> getPages();
	virtual string getText();

private:
	void swap(Control* control);

private:
	size_t pageNr;
	PageRef swappedPage;
	PageRef otherPage;
	bool moveUp;
};
