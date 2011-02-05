#include "InsertLayerUndoAction.h"
#include "../model/Layer.h"
#include "../model/Page.h"
#include "../model/Document.h"
#include "../control/Control.h"

InsertLayerUndoAction::InsertLayerUndoAction(XojPage * page, Layer * layer) {
	this->page = page;
	this->layer = layer;
}

InsertLayerUndoAction::~InsertLayerUndoAction() {
	if (undone) {
		// The layer was undone, also deleted
		delete this->layer;
	}
}

String InsertLayerUndoAction::getText() {
	return "Insert layer";
}

bool InsertLayerUndoAction::undo(Control * control) {
	this->page->removeLayer(this->layer);
	Document * doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = true;

	return true;
}

bool InsertLayerUndoAction::redo(Control * control) {
	this->page->addLayer(this->layer);
	Document * doc = control->getDocument();
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = false;

	return true;
}

