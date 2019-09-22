/*
 * Xournal++
 *
 * Undo action for delete (eraser, delete)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"
#include <XournalType.h>

class Element;
class Layer;
class Redrawable;

class DeleteUndoAction : public UndoAction
{
public:
	DeleteUndoAction(const PageRef& page, bool eraser);
	~DeleteUndoAction() override;

public:
	bool undo(Control*) override;
	bool redo(Control*) override;

	void addElement(Layer* layer, Element* e, int pos);

	string getText() override;

private:
	GList* elements = nullptr;
	bool eraser = true;
};
