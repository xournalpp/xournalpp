#include "ScaleUndoAction.h"

#include "../model/PageRef.h"
#include "../model/Element.h"
#include <Range.h>

ScaleUndoAction::ScaleUndoAction(PageRef page,
								 GList* elements,
								 double x0, double y0,
								 double fx, double fy) : UndoAction("ScaleUndoAction")
{
	XOJ_INIT_TYPE(ScaleUndoAction);

	this->page = page;
	this->elements = g_list_copy(elements);
	this->x0 = x0;
	this->y0 = y0;
	this->fx = fx;
	this->fy = fy;
}

ScaleUndoAction::~ScaleUndoAction()
{
	XOJ_CHECK_TYPE(ScaleUndoAction);

	this->page = NULL;
	g_list_free(this->elements);
	this->elements = NULL;

	XOJ_RELEASE_TYPE(ScaleUndoAction);
}

bool ScaleUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(ScaleUndoAction);

	applyScale(1 / this->fx, 1 / this->fy);
	this->undone = true;
	return true;
}

bool ScaleUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(ScaleUndoAction);

	applyScale(this->fx, this->fy);
	this->undone = false;
	return true;
}

void ScaleUndoAction::applyScale(double fx, double fy)
{
	XOJ_CHECK_TYPE(ScaleUndoAction);

	if (this->elements == NULL)
	{
		return;
	}

	Element* first = (Element*) elements->data;
	Range r(first->getX(), first->getY());

	for (GList* l = this->elements; l != NULL; l = l->next)
	{
		Element* e = (Element*) l->data;
		r.addPoint(e->getX(), e->getY());
		r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
		e->scale(this->x0, this->y0, fx, fy);
		r.addPoint(e->getX(), e->getY());
		r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
	}

	this->page->fireRangeChanged(r);
}

string ScaleUndoAction::getText()
{
	XOJ_CHECK_TYPE(ScaleUndoAction);

	return _("Scale");
}
