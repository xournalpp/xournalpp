#include "EraseHandler.h"
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
		this->cleanup();
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

		if (!s->isCopyed()) {
			Stroke * copy = s->clone();
			eraseUndoAction->addOriginal(l, s, pos);
			eraseUndoAction->addEdited(l, copy, pos);
			copy->setCopyed(true);

			// Because of undo / redo handling:
			// Remove the original and add the copy
			// if we undo this we need the original on the layer, else it can not be identified
			int spos = l->removeElement(s, false);
			l->insertElement(copy, spos);
			s = copy;
		}

		Point removedPoint(NAN, NAN);

		Stroke * part = s->splitOnLastIntersects(removedPoint);
		if (s->getPointCount() == 0) {
			eraseUndoAction->removeEdited(s);
			l->removeElement(s, true);
			this->view->repaint(repaintX, repaintY, repaintWidth, repaintHeight);
			return;
		}

		if (part) {
			l->insertElement(part, pos);
			eraseUndoAction->addEdited(l, part, pos);
			part->setCopyed(true);
		}

		Point first = s->getPoint(s->getPointCount() - 1);
		Point second;
		if (part) {
			second = part->getPoint(0);
		} else {
			second = removedPoint;
		}
		if (!isnan(second.x)) {
			double space = first.lineLengthTo(second);
			double toEraser = first.lineLengthTo(Point(x, y));
			if (space > 1.2 * halfEraserSize) {
				// TODO: this is not finished...
				printf("test %lf %lf\n", space, halfEraserSize);

				if (toEraser - halfEraserSize > space) {
					s->addPoint(second);
				} else {
					s->addPoint(first.lineTo(second, toEraser - halfEraserSize));
				}
			}
		}

		this->view->repaint(repaintX, repaintY, repaintWidth, repaintHeight);
	}

	printf("\n\n\n\n");

}

void EraseHandler::cleanup() {
	if (this->eraseUndoAction) {
		this->eraseUndoAction->cleanup();
		ListIterator<Layer*> pit = page->layerIterator();
		while (pit.hasNext()) {
			Layer * l = pit.next();
			ListIterator<Element *> lit = l->elementIterator();
			while (lit.hasNext()) {
				Element * e = lit.next();
				if (e->getType() == ELEMENT_STROKE) {
					Stroke * s = (Stroke *) e;
					s->setCopyed(false);
					s->freeUnusedPointItems();
				}
			}
		}

		this->eraseUndoAction = NULL;
	}
}
