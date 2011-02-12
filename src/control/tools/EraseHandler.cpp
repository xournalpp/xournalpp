#include "EraseHandler.h"
#include "../../model/EraseableStroke.h"
#include <math.h>

EraseHandler::EraseHandler(UndoRedoHandler * undo, XojPage * page, ToolHandler * handler, Redrawable * view) {
	this->page = page;
	this->handler = handler;
	this->view = view;

	this->eraseDeleteUndoAction = NULL;
	this->eraseUndoAction = NULL;
	this->undo = undo;

	this->halfEraserSize = 0;
}

EraseHandler::~EraseHandler() {
	if (this->eraseDeleteUndoAction) {
		this->finalize();
	}
}

/**
 * Handle eraser event: Delete Stroke and Standard, Whiteout is not handled here
 */
void EraseHandler::erase(double x, double y) {
	ListIterator<Layer*> it = this->page->layerIterator();

	int selected = page->getSelectedLayerId();

	this->halfEraserSize = this->handler->getThikness();
	GdkRectangle eraserRect = { x - halfEraserSize, y - halfEraserSize, halfEraserSize * 2, halfEraserSize * 2 };

	while (it.hasNext() && selected) {
		Layer * l = it.next();

		ListIterator<Element *> eit = l->elementIterator();
		eit.freeze();
		while (eit.hasNext()) {
			Element * e = eit.next();
			if (e->getType() == ELEMENT_STROKE && e->intersectsArea(&eraserRect)) {
				Stroke * s = (Stroke *) e;
				eraseStroke(l, s, x, y);
			}
		}

		selected--;
	}
}

void EraseHandler::eraseStroke(Layer * l, Stroke * s, double x, double y) {
	if (!s->intersects(x, y, halfEraserSize)) {
		return;
	}

	// delete complete element
	if (this->handler->getEraserType() == ERASER_TYPE_DELETE_STROKE) {
		int pos = l->removeElement(s, false);
		if (pos == -1) {
			return;
		}
		this->view->repaint(s);

		if (!this->eraseDeleteUndoAction) {
			this->eraseDeleteUndoAction = new DeleteUndoAction(this->page, this->view, true);
			this->undo->addUndoAction(this->eraseDeleteUndoAction);
		}

		this->eraseDeleteUndoAction->addElement(l, s, pos);
	} else { // Default eraser
		int pos = l->indexOf(s);
		if (pos == -1) {
			return;
		}

		if (eraseUndoAction == NULL) {
			eraseUndoAction = new EraseUndoAction(this->page, this->view);
			this->undo->addUndoAction(eraseUndoAction);
		}

		double repaintX = s->getX();
		double repaintY = s->getY();
		double repaintWidth = s->getElementWidth();
		double repaintHeight = s->getElementHeight();

		EraseableStroke * eraseable = NULL;
		if (s->getEraseable() == NULL) {
			eraseable = new EraseableStroke(s);
			s->setEraseable(eraseable);
			eraseUndoAction->addOriginal(l, s, pos);
		} else {
			eraseable = s->getEraseable();
		}

		if (eraseable->erase(x, y, halfEraserSize)) {
			this->view->repaint(s);
		}
	}
}

void EraseHandler::finalize() {
	if (this->eraseUndoAction) {
		this->eraseUndoAction->finalize();
		this->eraseUndoAction = NULL;
	}
}
