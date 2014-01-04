#include "SizeUndoAction.h"

#include "../model/Stroke.h"
#include <Range.h>
#include "../gui/Redrawable.h"

class SizeUndoActionEntry
{
public:
	SizeUndoActionEntry(Stroke* s, double orignalWidth, double newWidth,
	                    double* originalPressure,
	                    double* newPressure, int pressureCount)
	{
		XOJ_INIT_TYPE(SizeUndoActionEntry);

		this->s = s;
		this->orignalWidth = orignalWidth;
		this->newWidth = newWidth;
		this->originalPressure = originalPressure;
		this->newPressure = newPressure;
		this->pressureCount = pressureCount;
	}

	~SizeUndoActionEntry()
	{
		XOJ_CHECK_TYPE(SizeUndoActionEntry);

		delete this->originalPressure;
		this->originalPressure = NULL;
		delete this->newPressure;
		this->newPressure = NULL;

		XOJ_RELEASE_TYPE(SizeUndoActionEntry);
	}

	XOJ_TYPE_ATTRIB;

	Stroke* s;
	double orignalWidth;
	double newWidth;

	double* originalPressure;
	double* newPressure;
	int pressureCount;
};

SizeUndoAction::SizeUndoAction(PageRef page, Layer* layer) : UndoAction("SizeUndoAction")
{
	XOJ_INIT_TYPE(SizeUndoAction);

	this->page = page;
	this->layer = layer;
	this->data = NULL;
}

SizeUndoAction::~SizeUndoAction()
{
	XOJ_CHECK_TYPE(SizeUndoAction);

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		SizeUndoActionEntry* e = (SizeUndoActionEntry*) l->data;
		delete e;
	}

	g_list_free(this->data);
	this->data = NULL;

	XOJ_RELEASE_TYPE(SizeUndoAction);
}

double* SizeUndoAction::getPressure(Stroke* s)
{
	int count = s->getPointCount();
	double* data = new double[count];
	for (int i = 0; i < count; i++)
	{
		data[i] = s->getPoint(i).z;
	}

	return data;
}

void SizeUndoAction::addStroke(Stroke* s, double originalWidth, double newWidt,
                               double* originalPressure,
                               double* newPressure, int pressureCount)
{
	XOJ_CHECK_TYPE(SizeUndoAction);

	this->data = g_list_append(this->data, new SizeUndoActionEntry(s, originalWidth,
	                                                               newWidt, originalPressure,
	                                                               newPressure, pressureCount));
}

bool SizeUndoAction::undo(Control* control)
{
	XOJ_CHECK_TYPE(SizeUndoAction);

	if (this->data == NULL)
	{
		return true;
	}

	SizeUndoActionEntry* e = (SizeUndoActionEntry*) this->data->data;
	Range range(e->s->getX(), e->s->getY());

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		SizeUndoActionEntry* e = (SizeUndoActionEntry*) l->data;
		e->s->setWidth(e->orignalWidth);
		e->s->setPressure(e->originalPressure);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX() + e->s->getElementWidth(),
		               e->s->getY() + e->s->getElementHeight());
	}

	this->page->fireRangeChanged(range);

	return true;
}

bool SizeUndoAction::redo(Control* control)
{
	XOJ_CHECK_TYPE(SizeUndoAction);

	if (this->data == NULL)
	{
		return true;
	}

	SizeUndoActionEntry* e = (SizeUndoActionEntry*) this->data->data;
	Range range(e->s->getX(), e->s->getY());

	for (GList* l = this->data; l != NULL; l = l->next)
	{
		SizeUndoActionEntry* e = (SizeUndoActionEntry*) l->data;
		e->s->setWidth(e->newWidth);
		e->s->setPressure(e->newPressure);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX() + e->s->getElementWidth(),
		               e->s->getY() + e->s->getElementHeight());
	}

	this->page->fireRangeChanged(range);

	return true;
}

String SizeUndoAction::getText()
{
	XOJ_CHECK_TYPE(SizeUndoAction);

	return _("Change stroke width");
}
