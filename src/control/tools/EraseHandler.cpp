#include "EraseHandler.h"
#include "../../model/eraser/EraseableStroke.h"
#include "../../model/Layer.h"
#include "../../model/Stroke.h"
#include "../../model/Document.h"
#include <Range.h>
#include <Rectangle.h>
#include "../../undo/UndoRedoHandler.h"
#include "../../undo/EraseUndoAction.h"
#include "../../undo/DeleteUndoAction.h"
#include "../../gui/PageView.h"
#include "../ToolHandler.h"
#include "../../control/jobs/RenderJob.h"

#include "../../gui/XournalView.h"

#include <math.h>

EraseHandler::EraseHandler(UndoRedoHandler * undo, Document * doc, PageRef page, ToolHandler * handler, Redrawable * view) {
	XOJ_INIT_TYPE(EraseHandler);

	this->page = page;
	this->handler = handler;
	this->view = view;
	this->doc = doc;

	this->eraseDeleteUndoAction = NULL;
	this->eraseUndoAction = NULL;
	this->undo = undo;

	this->halfEraserSize = 0;
}

EraseHandler::~EraseHandler() {
	XOJ_CHECK_TYPE(EraseHandler);

	if (this->eraseDeleteUndoAction) {
		this->finalize();
	}

	XOJ_RELEASE_TYPE(EraseHandler);
}

/**
 * Handle eraser event: "Delete Stroke" and "Standard", Whiteout is not handled here
 */
void EraseHandler::erase(double x, double y) {
	XOJ_CHECK_TYPE(EraseHandler);

	this->halfEraserSize = this->handler->getThickness();
	GdkRectangle eraserRect = { x - halfEraserSize, y - halfEraserSize, halfEraserSize * 2, halfEraserSize * 2 };

	Range * range = new Range(x, y);

	Layer * l = page.getSelectedLayer();

	ListIterator<Element *> eit = l->elementIterator();
	eit.freeze();
	while (eit.hasNext()) {
		Element * e = eit.next();
		if (e->getType() == ELEMENT_STROKE && e->intersectsArea(&eraserRect)) {
			Stroke * s = (Stroke *) e;

			eraseStroke(l, s, x, y, range);
		}
	}

	this->view->rerenderRange(*range);
	delete range;
}

void EraseHandler::eraseStroke(Layer * l, Stroke * s, double x, double y, Range * range) {
	XOJ_CHECK_TYPE(EraseHandler);

	if (!s->intersects(x, y, halfEraserSize)) {
		return;
	}

	// delete complete element
	if (this->handler->getEraserType() == ERASER_TYPE_DELETE_STROKE) {
		this->doc->lock();
		int pos = l->removeElement(s, false);
		this->doc->unlock();

		if (pos == -1) {
			return;
		}
		range->addPoint(s->getX(), s->getY());
		range->addPoint(s->getX() + s->getElementWidth(), s->getY() + s->getElementHeight());

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

		EraseableStroke * eraseable = NULL;
		if (s->getEraseable() == NULL) {
			doc->lock();
			eraseable = new EraseableStroke(s);
			s->setEraseable(eraseable);
			doc->unlock();
			eraseUndoAction->addOriginal(l, s, pos);
		} else {
			eraseable = s->getEraseable();
		}

		eraseable->erase(x, y, halfEraserSize, range);
	}
}

void EraseHandler::finalize() {
	XOJ_CHECK_TYPE(EraseHandler);

	if (this->eraseUndoAction) {
		this->eraseUndoAction->finalize();
		this->eraseUndoAction = NULL;
	}
}
