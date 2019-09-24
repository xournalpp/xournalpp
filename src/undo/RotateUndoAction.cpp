#include "RotateUndoAction.h"

#include "model/Element.h"
#include "model/PageRef.h"

#include <i18n.h>
#include <Range.h>

RotateUndoAction::RotateUndoAction(PageRef page, vector<Element*>* elements, double x0, double y0, double xo, double yo, double rotation)
 : UndoAction("RotateUndoAction")
{
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
	this->page = nullptr;
}

bool RotateUndoAction::undo(Control* control)
{
	applyRotation(-this->rotation);
	this->undone = true;
	return true;
}

bool RotateUndoAction::redo(Control* control)
{
	applyRotation(this->rotation);
	this->undone = false;
	return true;
}

void RotateUndoAction::applyRotation(double rotation)
{
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
	return _("Rotation");
}
