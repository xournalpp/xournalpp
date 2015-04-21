/*
 * Xournal++
 *
 * Undo action for color changes (Edit selection)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

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
