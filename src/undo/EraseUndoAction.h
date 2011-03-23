/*
 * Xournal++
 *
 * Undo action for eraser
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ERASEUNDOACTION_H__
#define __ERASEUNDOACTION_H__

#include "UndoAction.h"
#include "../util/XournalType.h"

class XojPage;
class Redrawable;
class Layer;
class Stroke;

class EraseUndoAction: public UndoAction {
public:
	EraseUndoAction(XojPage * page, Redrawable * view);
	~EraseUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);

	void addOriginal(Layer * layer, Stroke * element, int pos);
	void addEdited(Layer * layer, Stroke * element, int pos);
	void removeEdited(Stroke * element);

	void finalize();

	virtual String getText();
private:
	XOJ_TYPE_ATTRIB;


	Redrawable * view;

	GList * edited;
	GList * original;
};

#endif /* __ERASEUNDOACTION_H__ */
