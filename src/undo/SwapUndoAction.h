/*
 * Xournal++
 *
 * Undo page swap action
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __SWAPUNDOACTION_H__
#define __SWAPUNDOACTION_H__

#include "UndoAction.h"

#include "model/PageRef.h"

class Control;

class SwapUndoAction : public UndoAction
{
public:
	SwapUndoAction(int pageNr, bool moveUp,
				   PageRef swapped_page,
				   PageRef other_page);

	virtual ~SwapUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	XojPage** getPages();
	virtual string getText();

private:
	void swap(Control* control);

private:
	XOJ_TYPE_ATTRIB;

	int pageNr;
	PageRef swapped_page, other_page;
	bool moveUp;
};

#endif
