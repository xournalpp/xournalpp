#include "InsertUndoAction.h"
#include "../model/PageRef.h"
#include "../model/Layer.h"
#include "../model/Element.h"
#include "../gui/Redrawable.h"

InsertUndoAction::InsertUndoAction(PageRef page, Layer * layer,
		Element * element, Redrawable * view) : UndoAction("InsertUndoAction") {
	XOJ_INIT_TYPE(InsertUndoAction);

	this->page = page;
	this->layer = layer;
	this->element = element;
	this->view = view;
}

InsertUndoAction::~InsertUndoAction() {
	XOJ_CHECK_TYPE(InsertUndoAction);

	if (this->undone) {
		// Insert was undone, so this is not needed anymore
		delete this->element;
	}
	this->element = NULL;

	XOJ_RELEASE_TYPE(InsertUndoAction);
}

String InsertUndoAction::getText() {
	XOJ_CHECK_TYPE(InsertUndoAction);

	if (element->getType() == ELEMENT_STROKE) {
		return _("Draw stroke");
	} else if (element->getType() == ELEMENT_TEXT) {
		return _("Write text");
	} else if (element->getType() == ELEMENT_IMAGE) {
		return _("Insert image");
	} else if (element->getType() == ELEMENT_TEXIMAGE) {
		return _("Insert latex");
	} else {
		return NULL;
	}
}

bool InsertUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(InsertUndoAction);

	this->layer->removeElement(this->element, false);

	this->view->rerenderElement(this->element);

	this->undone = true;

	return true;
}

bool InsertUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(InsertUndoAction);

	this->layer->addElement(this->element);

	this->view->rerenderElement(this->element);

	this->undone = false;

	return true;
}
