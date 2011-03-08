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
	Document * doc = control->getDocument();

	doc->lock();

	this->page->removeLayer(this->layer);
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = true;

	doc->unlock();
	return true;
}

bool InsertLayerUndoAction::redo(Control * control) {
	Document * doc = control->getDocument();

	doc->lock();

	this->page->addLayer(this->layer);
	int id = doc->indexOf(this->page);
	control->getWindow()->getXournal()->layerChanged(id);

	control->getWindow()->updateLayerCombobox();

	this->undone = false;

	doc->unlock();

	return true;
}

