/*
 * Xournal++
 *
 * Undo action for stroke recognizer
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
class Stroke;

class RecognizerUndoAction : public UndoAction
{
public:
	RecognizerUndoAction(PageRef page, Layer* layer, Stroke* original, Stroke* recognized);
	virtual ~RecognizerUndoAction();

public:
	void addSourceElement(Stroke* s);

	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	Layer* layer;
	Stroke* recognized;
	std::vector<Stroke*> original;
};
