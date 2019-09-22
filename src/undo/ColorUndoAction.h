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

class ColorUndoActionEntry;
class Element;
class Layer;
class Redrawable;

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
	std::vector<ColorUndoActionEntry*> data;
	Layer* layer;
};
