#include "RotateUndoAction.h"

#include "model/Element.h"
#include "model/PageRef.h"

#include <i18n.h>
#include <Range.h>

RotateUndoAction::RotateUndoAction(PageRef page, ElementVector* elements, double x0, double y0, double xo, double yo, double rotation) :
		UndoAction("RotateUndoAction")
{
	XOJ_INIT_TYPE(RotateUndoAction);

	this->page = page;
	this->elements = *elements;
	this->x0 = x0;
	this->y0 = y0;
	this->xo = xo;
	this->yo = yo;
	this->rotation = rotation;
}

RotateUndoAction::~RotateUndoAction()
{
	XOJ_CHECK_TYPE(RotateUndoAction);

	this->page = NULL;

	XOJ_RELEASE_TYPE(RotateUndoAction);
}

bool RotateUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(RotateUndoAction);

	applyRotation(-this->rotation);
	this->undone = true;
	return true;
}

bool RotateUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(RotateUndoAction);

	applyRotation(this->rotation);
	this->undone = false;
	return true;
}

void RotateUndoAction::applyRotation(double rotation)
{
	XOJ_CHECK_TYPE(RotateUndoAction);

	if (this->elements.empty()) return;

	Range r(elements.front()->getX(), elements.front()->getY());

	for (Element* e : this->elements)
	{
		r.addPoint(e->getX(), e->getY());
		r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
		e->rotate(this->x0, this->y0, this->xo, this->yo,rotation);
		r.addPoint(e->getX(), e->getY());
		r.addPoint(e->getX() + e->getElementWidth(), e->getY() + e->getElementHeight());
	}

	this->page->fireRangeChanged(r);
}

string RotateUndoAction::getText()
{
	XOJ_CHECK_TYPE(RotateUndoAction);

	return _("Rotation");
}
