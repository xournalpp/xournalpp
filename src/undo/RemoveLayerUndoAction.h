/*
 * Xournal++
 *
 * Undo action for remove layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "UndoAction.h"

class Layer;

class RemoveLayerUndoAction : public UndoAction
{
public:
	RemoveLayerUndoAction(PageRef page, Layer* layer, int layerPos);
	virtual ~RemoveLayerUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	XOJ_TYPE_ATTRIB;

	Layer* layer;
	int layerPos;
};
