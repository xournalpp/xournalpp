#include "RecognizerUndoAction.h"

#include "../model/Layer.h"
#include "../model/Element.h"
#include "../gui/Redrawable.h"

RecognizerUndoAction::RecognizerUndoAction(XojPage * page, Redrawable * view, Layer * layer, Element * original,
		Element * recognized) {
	this->page = page;
	this->view = view;
	this->layer = layer;
	this->original = original;
	this->recognized = recognized;
}

RecognizerUndoAction::~RecognizerUndoAction() {
	if (undone) {
		delete original;
	} else {
		delete recognized;
	}
	original = NULL;
	recognized = NULL;
}

bool RecognizerUndoAction::undo(Control * control) {
	int pos = this->layer->removeElement(this->recognized, false);
	this->layer->insertElement(this->original, pos);

	this->view->repaint();

	undone = true;
	return true;
}

bool RecognizerUndoAction::redo(Control * control) {
	int pos = this->layer->removeElement(this->original, false);
	this->layer->insertElement(this->recognized, pos);

	this->view->repaint();

	undone = false;
	return true;
}

String RecognizerUndoAction::getText() {
	return _("Stroke recognizer");
}

