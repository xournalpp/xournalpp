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

class InsertLayerUndoAction : public UndoAction
{
public:
	InsertLayerUndoAction(LayerController* layerController, PageRef page, Layer* layer, int layerPosition);
	virtual ~InsertLayerUndoAction();

public:
	virtual bool undo(Control* control);
	virtual bool redo(Control* control);

	virtual string getText();

private:
	int layerPosition;
	LayerController* layerController;
	Layer* layer;
};
