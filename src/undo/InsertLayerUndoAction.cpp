#include "InsertLayerUndoAction.h"

#include "control/layer/LayerController.h"
#include "control/Control.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/Layer.h"
#include "model/PageRef.h"

#include <i18n.h>

InsertLayerUndoAction::InsertLayerUndoAction(LayerController* layerController, PageRef page, Layer* layer, int layerPosition)
 : UndoAction("InsertLayerUndoAction"),
   layerPosition(layerPosition),
   layerController(layerController)
{
	this->page = page;
	this->layer = layer;
}

InsertLayerUndoAction::~InsertLayerUndoAction()
{
	if (this->undone)
	{
		// The layer was undone, also deleted
		delete this->layer;
	}
}

string InsertLayerUndoAction::getText()
{
	return _("Insert layer");
}

bool InsertLayerUndoAction::undo(Control* control)
{
	// perform the same thing we did to InsertDeletePage
	// to prevent a double lock (we're already locked here)
	// doc->lock();

	layerController->removeLayer(this->page, this->layer);

	this->undone = true;

	return true;
}

bool InsertLayerUndoAction::redo(Control* control)
{
	layerController->insertLayer(this->page, this->layer, layerPosition);
	Document* doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	this->undone = false;

	return true;
}
