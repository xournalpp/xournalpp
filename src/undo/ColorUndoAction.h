/*
 * Xournal++
 *
 * Undo action for color changes (Edit selection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __COLORUNDOACTION_H__
#define __COLORUNDOACTION_H__

#include "UndoAction.h"
#include <XournalType.h>
#include <glib.h>

class Layer;
class Redrawable;
class Element;

class ColorUndoAction : public UndoAction
{
public:
	ColorUndoAction(PageRef page, Layer* layer);
	virtual ~ColorUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	virtual string getText();

	void addStroke(Element* e, int originalColor, double newColor);

private:
	XOJ_TYPE_ATTRIB;


	GList* data;

	Layer* layer;
};

#endif /* __COLORUNDOACTION_H__ */
