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
// TODO: AA: type check

#ifndef __INSERTUNDOACTION_H__
#define __INSERTUNDOACTION_H__

#include "UndoAction.h"

class XojPage;
class Layer;
class Element;
class Redrawable;

class InsertUndoAction: public UndoAction {
public:
	InsertUndoAction(XojPage * page, Layer * layer, Element * element, Redrawable * view);
	~InsertUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	virtual String getText();
private:
	Layer * layer;
	Element * element;
	Redrawable * view;
};

#endif /* __INSERTUNDOACTION_H__ */
