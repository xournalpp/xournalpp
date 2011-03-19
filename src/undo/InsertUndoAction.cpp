#include "InsertUndoAction.h"
#include "../model/Page.h"
#include "../model/Layer.h"
#include "../model/Element.h"
#include "../gui/Redrawable.h"

InsertUndoAction::InsertUndoAction(XojPage * page, Layer * layer, Element * element, Redrawable * view) {
	this->page = page;
	this->layer = layer;
	this->element = element;
	this->view = view;
}

InsertUndoAction::~InsertUndoAction() {
	if (undone) {
		// Insert was undone, so this is not needed anymore
		delete this->element;
	}
}

String InsertUndoAction::getText() {
	if (element->getType() == ELEMENT_STROKE) {
		return _("Draw stroke");
	} else if (element->getType() == ELEMENT_TEXT) {
		return _("Write text");
	} else if (element->getType() == ELEMENT_IMAGE) {
		return _("Insert image");
	} else {
		return NULL;
	}
}

bool InsertUndoAction::undo(Control * control) {
	this->layer->removeElement(this->element, false);

	view->rerenderElement(this->element);

	this->undone = true;

	return true;
}

bool InsertUndoAction::redo(Control * control) {
	this->layer->addElement(this->element);

	view->rerenderElement(this->element);

	this->undone = false;

	return true;
}
