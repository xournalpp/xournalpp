#include "EraseUndoAction.h"
#include "../gui/Redrawable.h"
#include "../model/Stroke.h"
#include "../model/Layer.h"
#include "../model/eraser/EraseableStroke.h"

#include "PageLayerPosEntry.h"

EraseUndoAction::EraseUndoAction(PageRef page, Redrawable * view) : UndoAction("EraseUndoAction") {
	XOJ_INIT_TYPE(EraseUndoAction);

	this->page = page;
	this->view = view;
	this->edited = NULL;
	this->original = NULL;
}

EraseUndoAction::~EraseUndoAction() {
	XOJ_CHECK_TYPE(EraseUndoAction);

	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		if (!undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->original);
	this->original = NULL;

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		if (undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->edited);
	this->edited = NULL;

	XOJ_RELEASE_TYPE(EraseUndoAction);
}

void EraseUndoAction::addOriginal(Layer * layer, Stroke * element, int pos) {
	XOJ_CHECK_TYPE(EraseUndoAction);

	this->original = g_list_insert_sorted(this->original, new PageLayerPosEntry<Stroke> (layer, element, pos),
			(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::addEdited(Layer * layer, Stroke * element, int pos) {
	XOJ_CHECK_TYPE(EraseUndoAction);

	this->edited = g_list_insert_sorted(this->edited, new PageLayerPosEntry<Stroke> (layer, element, pos),
			(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::removeEdited(Stroke * element) {
	XOJ_CHECK_TYPE(EraseUndoAction);

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * p = (PageLayerPosEntry<Stroke> *) l->data;
		if (p->element == element) {
			this->edited = g_list_delete_link(this->edited, l);
			delete p;
			return;
		}
	}
}

void EraseUndoAction::finalize() {
	XOJ_CHECK_TYPE(EraseUndoAction);

	for (GList * l = this->original; l != NULL;) {
		PageLayerPosEntry<Stroke> * p = (PageLayerPosEntry<Stroke> *) l->data;
		GList * del = l;
		l = l->next;

		if (p->element->getPointCount() == 0) {
			this->edited = g_list_delete_link(this->edited, del);
			delete p;
		} else {

			// Remove the original and add the copy
			int pos = p->layer->removeElement(p->element, false);

			EraseableStroke * e = p->element->getEraseable();
			GList * stroke = e->getStroke(p->element);
			for (GList * ls = stroke; ls != NULL; ls = ls->next) {
				Stroke * copy = (Stroke *) ls->data;
				p->layer->insertElement(copy, pos);
				this->addEdited(p->layer, copy, pos);
				pos++;
			}

			delete e;
			p->element->setEraseable(NULL);
		}
	}
	view->rerenderPage();
}

String EraseUndoAction::getText() {
	XOJ_CHECK_TYPE(EraseUndoAction);

	return _("Erase stroke");
}

bool EraseUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(EraseUndoAction);

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->removeElement(e->element, false);
		view->rerenderElement(e->element);
	}

	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->insertElement(e->element, e->pos);
		view->rerenderElement(e->element);
	}

	this->undone = true;
	return true;
}

bool EraseUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(EraseUndoAction);

	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->removeElement(e->element, false);
		view->rerenderElement(e->element);
	}

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;

		e->layer->insertElement(e->element, e->pos);
		view->rerenderElement(e->element);
	}

	this->undone = false;
	return true;
}
