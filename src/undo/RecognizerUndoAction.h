/*
 * Xournal++
 *
 * Undo action for stroke recognizer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "UndoAction.h"

class Redrawable;
class Stroke;
class Layer;

class RecognizerUndoAction : public UndoAction
{
public:
	RecognizerUndoAction(PageRef page, Layer* layer,
						Stroke* original, Stroke* recognized);
	virtual ~RecognizerUndoAction();

public:
	void addSourceElement(Stroke* s);

	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	XOJ_TYPE_ATTRIB;

	Layer* layer;
	Stroke* recognized;
	GList* original;
};
