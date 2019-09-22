#include "SizeUndoAction.h"

#include "gui/Redrawable.h"
#include "model/Stroke.h"

#include <i18n.h>
#include <Range.h>

class SizeUndoActionEntry
{
public:
	SizeUndoActionEntry(Stroke* s, double orignalWidth, double newWidth, vector<double> originalPressure,
						vector<double> newPressure, int pressureCount)
	{
		this->s = s;
		this->orignalWidth = orignalWidth;
		this->newWidth = newWidth;
		this->originalPressure = originalPressure;
		this->newPressure = newPressure;
		this->pressureCount = pressureCount;
	}

	~SizeUndoActionEntry()
	{
	}
	Stroke* s;
	double orignalWidth;
	double newWidth;

	vector<double> originalPressure;
	vector<double> newPressure;
	int pressureCount;
};

SizeUndoAction::SizeUndoAction(PageRef page, Layer* layer) : UndoAction("SizeUndoAction")
{
	this->page = page;
	this->layer = layer;
}

SizeUndoAction::~SizeUndoAction()
{
	for (SizeUndoActionEntry* e : this->data)
	{
		delete e;
	}
	this->data.clear();
}

vector<double> SizeUndoAction::getPressure(Stroke* s)
{
	int count = s->getPointCount();
	vector<double> data;
	for (int i = 0; i < count; i++)
	{
		data.push_back(s->getPoint(i).z);
	}

	return data;
}

void SizeUndoAction::addStroke(Stroke* s, double originalWidth, double newWidth, vector<double> originalPressure,
							   vector<double> newPressure, int pressureCount)
{
	this->data.push_back(new SizeUndoActionEntry(s, originalWidth, newWidth, originalPressure, newPressure, pressureCount));
}

bool SizeUndoAction::undo(Control* control)
{
	if (this->data.empty())
	{
		return true;
	}

	SizeUndoActionEntry* e = this->data.front();
	Range range(e->s->getX(), e->s->getY());

	for (SizeUndoActionEntry* e : this->data)
	{
		e->s->setWidth(e->orignalWidth);
		e->s->setPressure(e->originalPressure);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
	}

	this->page->fireRangeChanged(range);

	return true;
}

bool SizeUndoAction::redo(Control* control)
{
	if (this->data.empty())
	{
		return true;
	}

	SizeUndoActionEntry* e = this->data.front();
	Range range(e->s->getX(), e->s->getY());

	for (SizeUndoActionEntry* e : this->data)
	{
		e->s->setWidth(e->newWidth);
		e->s->setPressure(e->newPressure);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX() + e->s->getElementWidth(), e->s->getY() + e->s->getElementHeight());
	}

	this->page->fireRangeChanged(range);

	return true;
}

string SizeUndoAction::getText()
{
	return _("Change stroke width");
}
