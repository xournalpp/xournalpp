/*
 * Xournal++
 *
 * Undo action for color changes (Edit selection)
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __COLORUNDOACTION_H__
#define __COLORUNDOACTION_H__

#include "UndoAction.h"
#include "../util/XournalType.h"
#include <glib.h>

class Layer;
class Redrawable;
class Element;

class ColorUndoAction: public UndoAction {
public:
	ColorUndoAction(XojPage * page, Layer * layer, Redrawable * view);
	~ColorUndoAction();

	virtual bool undo(Control * control);
	virtual bool redo(Control * control);
	virtual String getText();

	void addStroke(Element * e, int originalColor, double newColor);

private:
	XOJ_TYPE_ATTRIB;


	GList * data;

	Layer * layer;
	Redrawable * view;
};

#endif /* __COLORUNDOACTION_H__ */
