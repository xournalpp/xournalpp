/*
 * Xournal++
 *
 * Undo action for delete (eraser, delete)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class Element;
class Layer;
class Redrawable;

class DeleteUndoAction : public UndoAction
{
public:
	DeleteUndoAction(PageRef page, bool eraser);
	virtual ~DeleteUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	void addElement(Layer* layer, Element* e, int pos);

	virtual string getText();

private:
	XOJ_TYPE_ATTRIB;

	GList* elements = NULL;
	bool eraser = true;
};
