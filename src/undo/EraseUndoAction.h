/*
 * Xournal++
 *
 * Undo action for eraser
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class Layer;
class Redrawable;
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
	GList* edited = nullptr;
	GList* original = nullptr;
};
