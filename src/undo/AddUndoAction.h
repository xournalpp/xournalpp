/*
 * Xournal++
 *
 * Undo action for delete (eraser, delete)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ADDUNDOACTION_H__
#define __ADDUNDOACTION_H__

#include "UndoAction.h"
#include <XournalType.h>

class Redrawable;
class Element;
class Layer;

class AddUndoAction: public UndoAction
{
public:
	AddUndoAction(PageRef page, bool eraser);
	virtual ~AddUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	void addElement(Layer* layer, Element* e, int pos);

	virtual String getText();

private:
	XOJ_TYPE_ATTRIB;

	GList* elements;
	bool eraser;
};

#endif /* __ADDUNDOACTION_H__ */
