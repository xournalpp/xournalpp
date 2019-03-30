#include "EraseHandler.h"

#include "control/jobs/RenderJob.h"
#include "control/ToolHandler.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/Document.h"
#include "model/eraser/EraseableStroke.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "undo/EraseUndoAction.h"
#include "undo/DeleteUndoAction.h"
#include "undo/UndoRedoHandler.h"

#include <Range.h>
#include <Rectangle.h>

#include <cmath>

EraseHandler::EraseHandler(UndoRedoHandler* undo, Document* doc, PageRef page, ToolHandler* handler, Redrawable* view)
{
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

EraseHandler::~EraseHandler()
{
	XOJ_CHECK_TYPE(EraseHandler);

	if (this->eraseDeleteUndoAction)
	{
		this->finalize();
	}

	XOJ_RELEASE_TYPE(EraseHandler);
}

/**
 * Handle eraser event: "Delete Stroke" and "Standard", Whiteout is not handled here
 */
void EraseHandler::erase(double x, double y)
{
	XOJ_CHECK_TYPE(EraseHandler);

	this->halfEraserSize = this->handler->getThickness();
	GdkRectangle eraserRect = {
		gint(x - halfEraserSize),
		gint(y - halfEraserSize),
		gint(halfEraserSize * 2),
		gint(halfEraserSize * 2)
	};

	Range* range = new Range(x, y);

	Layer* l = page->getSelectedLayer();

	vector<Element*> tmp(*l->getElements());
	for (Element* e : tmp)
	{
		if (e->getType() == ELEMENT_STROKE && e->intersectsArea(&eraserRect))
		{
			eraseStroke(l, (Stroke*) e, x, y, range);
		}
	}

	this->view->rerenderRange(*range);
	delete range;
}

void EraseHandler::eraseStroke(Layer* l, Stroke* s, double x, double y, Range* range)
{
	XOJ_CHECK_TYPE(EraseHandler);

	if (!s->intersects(x, y, halfEraserSize))
	{
		return;
	}

	// delete complete element
	if (this->handler->getEraserType() == ERASER_TYPE_DELETE_STROKE)
	{
		this->doc->lock();
		int pos = l->removeElement(s, false);
		this->doc->unlock();

		if (pos == -1)
		{
			return;
		}
		range->addPoint(s->getX(), s->getY());
		range->addPoint(s->getX() + s->getElementWidth(), s->getY() + s->getElementHeight());

		//removed the if statement - this prevents us from putting multiple elements into a
		//stroke erase operation, but it also prevents the crashing and layer issues!
		if (!this->eraseDeleteUndoAction)
		{
			this->eraseDeleteUndoAction = new DeleteUndoAction(this->page, true);

			this->undo->addUndoAction(this->eraseDeleteUndoAction);
		}

		this->eraseDeleteUndoAction->addElement(l, s, pos);
	}
	else // Default eraser
	{
		int pos = l->indexOf(s);
		if (pos == -1)
		{
			return;
		}

		if (eraseUndoAction == NULL)
		{
			eraseUndoAction = new EraseUndoAction(this->page);
			this->undo->addUndoAction(eraseUndoAction);
		}

		EraseableStroke* eraseable = NULL;
		if (s->getEraseable() == NULL)
		{
			doc->lock();
			eraseable = new EraseableStroke(s);
			s->setEraseable(eraseable);
			doc->unlock();
			eraseUndoAction->addOriginal(l, s, pos);
		}
		else
		{
			eraseable = s->getEraseable();
		}

		eraseable->erase(x, y, halfEraserSize, range);
	}
}

void EraseHandler::finalize()
{
	XOJ_CHECK_TYPE(EraseHandler);

	if (this->eraseUndoAction)
	{
		this->eraseUndoAction->finalize();
		this->eraseUndoAction = NULL;
	}
	else if (this->eraseDeleteUndoAction)
	{
		this->eraseDeleteUndoAction = NULL;
	}
}
