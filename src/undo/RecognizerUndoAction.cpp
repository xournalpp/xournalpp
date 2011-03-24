#include "RecognizerUndoAction.h"

#include "../model/Layer.h"
#include "../model/Stroke.h"
#include "../gui/Redrawable.h"
#include "../util/Stacktrace.h"

RecognizerUndoAction::RecognizerUndoAction(XojPage * page, Redrawable * view, Layer * layer, Stroke * original, Stroke * recognized) {
	XOJ_INIT_TYPE(RecognizerUndoAction);

	this->page = page;
	this->view = view;
	this->layer = layer;
	this->original = NULL;
	this->recognized = recognized;

	addSourceElement(original);
}

RecognizerUndoAction::~RecognizerUndoAction() {
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	if (this->undone) {
		delete this->recognized;
	} else {
		for (GList * l = this->original; l != NULL; l = l->next) {
			Stroke * s = (Stroke *) l->data;
			delete s;
		}
	}
	this->original = NULL;
	this->recognized = NULL;

	XOJ_RELEASE_TYPE(RecognizerUndoAction);
}

void RecognizerUndoAction::addSourceElement(Stroke * s) {
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	GList * elem2 = g_list_find(this->original, s);
	if (elem2) {
		g_warning("RecognizerUndoAction::addSourceElement() twice the same\n");
		Stacktrace::printStracktrace();
		return;
	}

	this->original = g_list_append(this->original, s);
}

bool RecognizerUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	int pos = this->layer->removeElement(this->recognized, false);
	this->view->rerenderElement(this->recognized);
	int i = 0;
	for (GList * l = this->original; l != NULL; l = l->next) {
		Stroke * s = (Stroke *) l->data;
		this->layer->insertElement(s, pos);
		this->view->rerenderElement(s);
		i++;
	}

	this->undone = true;
	return true;
}

bool RecognizerUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	int pos = 0;
	for (GList * l = this->original; l != NULL; l = l->next) {
		Stroke * s = (Stroke *) l->data;
		pos = this->layer->removeElement(s, false);
		this->view->rerenderElement(s);
	}
	this->layer->insertElement(this->recognized, pos);

	this->view->rerenderElement(this->recognized);

	this->undone = false;
	return true;
}

String RecognizerUndoAction::getText() {
	XOJ_CHECK_TYPE(RecognizerUndoAction);

	return _("Stroke recognizer");
}

