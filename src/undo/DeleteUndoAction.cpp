#include "DeleteUndoAction.h"
#include "../model/Layer.h"
#include "../model/Element.h"
#include "../model/PageRef.h"
#include "../gui/Redrawable.h"
#include "PageLayerPosEntry.h"

DeleteUndoAction::DeleteUndoAction(PageRef page, Redrawable * view, bool eraser) : UndoAction("DeleteUndoAction") {
	XOJ_INIT_TYPE(DeleteUndoAction);

	this->page = page;
	this->view = view;
	this->eraser = eraser;
	this->elements = NULL;
}

DeleteUndoAction::~DeleteUndoAction() {
	XOJ_CHECK_TYPE(DeleteUndoAction);

	for (GList * l = this->elements; l != NULL; l = l->next) {
		PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
		if (!undone) {
			delete e->element;
		}
		delete e;
	}
	g_list_free(this->elements);

	XOJ_RELEASE_TYPE(DeleteUndoAction)
	;
}

void DeleteUndoAction::addElement(Layer * layer, Element * e, int pos) {
	XOJ_CHECK_TYPE(DeleteUndoAction);

	this->elements = g_list_insert_sorted(this->elements, new PageLayerPosEntry<Element> (layer, e, pos), (GCompareFunc) PageLayerPosEntry<Element>::cmp);
}

bool DeleteUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(DeleteUndoAction);

	if (this->elements == NULL) {
		g_warning("Could not undo DeleteUndoAction, there is nothing to undo");

		this->undone = true;
		return false;
	}

	for (GList * l = this->elements; l != NULL; l = l->next) {
		PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->insertElement(e->element, e->pos);
		view->rerenderElement(e->element);
	}

	this->undone = true;
	return true;
}

bool DeleteUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(DeleteUndoAction);

	if (this->elements == NULL) {
		g_warning("Could not redo DeleteUndoAction, there is nothing to redo");

		this->undone = false;
		return false;
	}

	for (GList * l = this->elements; l != NULL; l = l->next) {
		PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
		e->layer->removeElement(e->element, false);
		view->rerenderElement(e->element);
	}

	this->undone = false;

	return true;
}

String DeleteUndoAction::getText() {
	XOJ_CHECK_TYPE(DeleteUndoAction);

	String text;

	if (eraser) {
		text = _("Erase stroke");
	} else {
		text = _("Delete");

		if (this->elements != NULL) {
			ElementType type = ((PageLayerPosEntry<Element>*) this->elements->data)->element->getType();

			for (GList * l = this->elements->next; l != NULL; l = l->next) {
				PageLayerPosEntry<Element> * e = (PageLayerPosEntry<Element>*) l->data;
				if (type != e->element->getType()) {
					text += _(" elements");
					return text;
				}
			}

			if (type == ELEMENT_STROKE) {
				text += _(" stroke");
			} else if (type == ELEMENT_TEXT) {
				text += _(" text");
			} else if (type == ELEMENT_IMAGE) {
				text += _(" image");
			}
		}
	}
	return text;
}
