/*
 * Xournal++
 *
 * Undo action for insert  layer
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
class LayerController;

class MoveLayerUndoAction : public UndoAction
{
public:
	MoveLayerUndoAction(LayerController* layerController, PageRef page, Layer* layer, int oldLayerPos, int newLayerPos);
	virtual ~MoveLayerUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	int oldLayerPos;
	int newLayerPos;
	LayerController* layerController;
	Layer* layer;
};
