/*
 * Xournal++
 *
 * Undo action for delete (eraser, delete)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class Redrawable;
class Element;
class Layer;

class AddUndoAction : public UndoAction
{
public:
	AddUndoAction(PageRef page, bool eraser);
	virtual ~AddUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	void addElement(Layer* layer, Element* e, int pos);

	virtual string getText();

private:
	XOJ_TYPE_ATTRIB;

	GList* elements;
	bool eraser;
};
