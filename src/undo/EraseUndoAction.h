/*
 * Xournal++
 *
 * Undo action for eraser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class Redrawable;
class Layer;
class Stroke;

class EraseUndoAction : public UndoAction
{
public:
	EraseUndoAction(PageRef page);
	virtual ~EraseUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	void addOriginal(Layer* layer, Stroke* element, int pos);
	void addEdited(Layer* layer, Stroke* element, int pos);
	void removeEdited(Stroke* element);

	void finalize();

	virtual string getText();
private:
	XOJ_TYPE_ATTRIB;

	GList* edited;
	GList* original;
};
