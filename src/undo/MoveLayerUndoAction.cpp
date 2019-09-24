#include "MoveLayerUndoAction.h"

#include "control/layer/LayerController.h"
#include "control/Control.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include <i18n.h>

MoveLayerUndoAction::MoveLayerUndoAction(LayerController* layerController, PageRef page, Layer* layer, int oldLayerPos, int newLayerPos)
 : UndoAction("MoveLayerUndoAction"),
   oldLayerPos(oldLayerPos),
   newLayerPos(newLayerPos),
   layerController(layerController)
{
	this->page = page;
	this->layer = layer;
}

MoveLayerUndoAction::~MoveLayerUndoAction()
{
	this->layerController = nullptr;
	this->layer = nullptr;
}

string MoveLayerUndoAction::getText()
{
	return _("Move layer");
}

bool MoveLayerUndoAction::undo(Control* control)
{
	layerController->removeLayer(this->page, this->layer);
	layerController->insertLayer(this->page, this->layer, oldLayerPos);

	this->undone = true;

	return true;
}

bool MoveLayerUndoAction::redo(Control* control)
{
	layerController->removeLayer(this->page, this->layer);
	layerController->insertLayer(this->page, this->layer, newLayerPos);

	this->undone = false;

	return true;
}
