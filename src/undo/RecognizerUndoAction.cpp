#include "RecognizerUndoAction.h"

#include "../model/Layer.h"
#include "../model/Stroke.h"
#include "../gui/Redrawable.h"
#include "../util/Stacktrace.h"

RecognizerUndoAction::RecognizerUndoAction(XojPage * page, Redrawable * view, Layer * layer, Stroke * original, Stroke * recognized) {
	this->page = page;
	this->view = view;
	this->layer = layer;
	this->original = NULL;
	this->recognized = recognized;

	addSourceElement(original);
}

RecognizerUndoAction::~RecognizerUndoAction() {
	if (undone) {
		delete recognized;
	} else {
		for (GList * l = this->original; l != NULL; l = l->next) {
			Stroke * s = (Stroke *) l->data;
			delete s;
		}
	}
	original = NULL;
	recognized = NULL;
}

void RecognizerUndoAction::addSourceElement(Stroke * s) {
	GList * elem2 = g_list_find(this->original, s);
	if (elem2) {
		g_warning("RecognizerUndoAction::addSourceElement() twice the same\n");
		Stacktrace::printStracktrace();
		return;
	}

	this->original = g_list_append(this->original, s);
}

bool RecognizerUndoAction::undo(Control * control) {
	int pos = this->layer->removeElement(this->recognized, false);
	int i = 0;
	for (GList * l = this->original; l != NULL; l = l->next) {
		Stroke * s = (Stroke *) l->data;
		this->layer->insertElement(s, pos);
		i++;
	}

	this->view->repaint();

	undone = true;
	return true;
}

bool RecognizerUndoAction::redo(Control * control) {
	int pos = 0;
	for (GList * l = this->original; l != NULL; l = l->next) {
		Stroke * s = (Stroke *) l->data;
		pos = this->layer->removeElement(s, false);
	}
	this->layer->insertElement(this->recognized, pos);

	this->view->repaint();

	undone = false;
	return true;
}

String RecognizerUndoAction::getText() {
	return _("Stroke recognizer");
}

