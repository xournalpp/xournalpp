/*
 * Xournal++
 *
 * Undo action for insert (write text, draw stroke...)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __INSERTUNDOACTION_H__
#define __INSERTUNDOACTION_H__

#include "UndoAction.h"

class Layer;
class Element;
class Redrawable;

class InsertUndoAction: public UndoAction {
public:
	InsertUndoAction(PageRef page, Layer * layer, Element * element, Redrawable * view);
	virtual ~InsertUndoAction();

public:
	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();

private:
	XOJ_TYPE_ATTRIB;

	Layer * layer;
	Element * element;
	Redrawable * view;
};

#endif /* __INSERTUNDOACTION_H__ */
