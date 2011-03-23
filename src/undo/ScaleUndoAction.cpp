#include "ScaleUndoAction.h"

#include "../model/Page.h"
#include "../model/Element.h"
// TODO: AA: type check

ScaleUndoAction::ScaleUndoAction(XojPage * page, Redrawable * view, GList * elements, double x0, double y0, double fx,
		double fy) {
	this->page = page;
	this->view = view;
	this->elements = g_list_copy(elements);
	this->page->reference();
	this->x0 = x0;
	this->y0 = y0;
	this->fx = fx;
	this->fy = fy;
}

ScaleUndoAction::~ScaleUndoAction() {
	this->page->unreference();
	this->page = NULL;
	this->view = NULL;
	g_list_free(this->elements);
}

bool ScaleUndoAction::undo(Control * control) {
	applayScale(1 / this->fx, 1 / this->fy);
	this->undone = true;
	return true;
}

bool ScaleUndoAction::redo(Control * control) {
	applayScale(this->fx, this->fy);
	this->undone = false;
	return true;
}

void ScaleUndoAction::applayScale(double fx, double fy) {
	for (GList * l = this->elements; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		e->scale(this->x0, this->y0, fx, fy);
	}
}

String ScaleUndoAction::getText() {
	return _("Scale");
}
