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

#ifndef __DELETEUNDOACTION_H__
#define __DELETEUNDOACTION_H__

#include "UndoAction.h"

class XojPage;
class Redrawable;
class Element;
class Layer;

class DeleteUndoAction: public UndoAction {
public:
	DeleteUndoAction(XojPage * page, Redrawable * view, bool eraser);
	~DeleteUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	void addElement(Layer * layer, Element * e, int pos);

	virtual String getText();
private:
	GList * elements;
	Redrawable * view;
	bool eraser;
};

#endif /* __DELETEUNDOACTION_H__ */
