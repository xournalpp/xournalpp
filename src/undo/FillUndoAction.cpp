#include "FillUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Stroke.h"

#include <i18n.h>
#include <Range.h>

class FillUndoActionEntry
{
public:
	FillUndoActionEntry(Stroke* s, int originalFill, int newFill)
	{
		this->s = s;
		this->originalFill = originalFill;
		this->newFill = newFill;
	}

	~FillUndoActionEntry()
	{
	}
	Stroke* s;
	int originalFill;
	int newFill;
};

FillUndoAction::FillUndoAction(PageRef page, Layer* layer)
 : UndoAction("FillUndoAction")
{
	this->page = page;
	this->layer = layer;
}

FillUndoAction::~FillUndoAction()
{
	for (FillUndoActionEntry* e : this->data)
	{
		delete e;
	}
	this->data.clear();
}

void FillUndoAction::addStroke(Stroke* s, int originalFill, int newFill)
{
	this->data.push_back(new FillUndoActionEntry(s, originalFill, newFill));
}

bool FillUndoAction::undo(Control* control)
{
	if (this->data.empty())
	{
		return true;
	}

	FillUndoActionEntry* e = this->data.front();
	Range range(e->s->getX(), e->s->getY());

	for (FillUndoActionEntry* e : this->data)
	{
		e->s->setFill(e->originalFill);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
	}

	this->page->fireRangeChanged(range);

	return true;
}

bool FillUndoAction::redo(Control* control)
{
	if (this->data.empty())
	{
		return true;
	}

	FillUndoActionEntry* e = this->data.front();
	Range range(e->s->getX(), e->s->getY());

	for (FillUndoActionEntry* e : this->data)
	{
		e->s->setFill(e->newFill);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
	}

	this->page->fireRangeChanged(range);

	return true;
}

string FillUndoAction::getText()
{
	return _("Change stroke fill");
}
