/*
 * Xournal++
 *
 * Undo action for remove layer
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "UndoAction.h"

class Layer;
class LayerController;

class RemoveLayerUndoAction : public UndoAction
{
public:
	RemoveLayerUndoAction(LayerController* layerController, PageRef page, Layer* layer, int layerPos);
	virtual ~RemoveLayerUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	LayerController* layerController;
	Layer* layer;
	int layerPos;
};
