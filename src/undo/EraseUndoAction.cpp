#include "EraseUndoAction.h"
#include "../gui/Redrawable.h"
#include "../model/Stroke.h"
#include "../model/Layer.h"

#include "PageLayerPosEntry.h"

EraseUndoAction::EraseUndoAction(XojPage * page, Redrawable * view) {
	this->page = page;
	this->view = view;
	this->edited = NULL;
	this->original = NULL;
}

EraseUndoAction::~EraseUndoAction() {
	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		if (!undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->original);

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		if (undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->edited);
}

void EraseUndoAction::addOriginal(Layer * layer, Stroke * element, int pos) {
	this->original = g_list_insert_sorted(this->original, new PageLayerPosEntry<Stroke> (layer, element, pos),
			(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::addEdited(Layer * layer, Stroke * element, int pos) {
	this->edited = g_list_insert_sorted(this->edited, new PageLayerPosEntry<Stroke> (layer, element, pos),
			(GCompareFunc) PageLayerPosEntry<Stroke>::cmp);
}

void EraseUndoAction::removeEdited(Stroke * element) {
	for (GList * l = this->edited; l != NULL;) {
		PageLayerPosEntry<Stroke> * p = (PageLayerPosEntry<Stroke> *) l->data;
		if (p->element == element) {
			g_list_remove_link(this->edited, l);
			delete p;
			return;
		}
	}
}

void EraseUndoAction::cleanup() {
	for (GList * l = this->edited; l != NULL;) {
		PageLayerPosEntry<Stroke> * p = (PageLayerPosEntry<Stroke> *) l->data;
		if (p->element->getPointCount() == 0) {
			GList * del = l;
			l = l->next;
			this->edited = g_list_remove_link(this->edited, del);
			continue;
		}

		l = l->next;
	}
}

String EraseUndoAction::getText() {
	return _("Erase stroke");
}

bool EraseUndoAction::undo(Control * control) {
	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->removeElement(e->element, false);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	this->view->repaint();

	this->undone = true;
	return true;
}

bool EraseUndoAction::redo(Control * control) {
	for (GList * l = this->original; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->removeElement(e->element, false);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	for (GList * l = this->edited; l != NULL; l = l->next) {
		PageLayerPosEntry<Stroke> * e = (PageLayerPosEntry<Stroke>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		view->repaint(e->element->getX(), e->element->getY(), e->element->getElementWidth(),
				e->element->getElementHeight());
	}

	this->view->repaint();

	this->undone = false;
	return true;

}
