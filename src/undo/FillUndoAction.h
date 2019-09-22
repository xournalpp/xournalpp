/*
 * Xournal++
 *
 * Undo action resize
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Layer;
class Redrawable;
class FillUndoActionEntry;
class Stroke;

class FillUndoAction : public UndoAction
{
public:
	FillUndoAction(PageRef page, Layer* layer);
	virtual ~FillUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);
	virtual string getText();

	void addStroke(Stroke* s, int originalFill, int newFill);

private:
	std::vector<FillUndoActionEntry*> data;

	Layer* layer;
};
