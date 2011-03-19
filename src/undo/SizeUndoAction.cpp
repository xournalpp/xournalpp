#include "SizeUndoAction.h"

#include "../model/Stroke.h"
#include "../util/Range.h"
#include "../gui/Redrawable.h"

class SizeUndoActionEntry {
public:
	SizeUndoActionEntry(Stroke * s, double orignalWidth, double newWidth, double * originalPressure,
			double * newPressure, int pressureCount) {
		this->s = s;
		this->orignalWidth = orignalWidth;
		this->newWidth = newWidth;
		this->originalPressure = originalPressure;
		this->newPressure = newPressure;
		this->pressureCount = pressureCount;
	}

	~SizeUndoActionEntry() {
		delete this->originalPressure;
		delete this->newPressure;
	}

	Stroke * s;
	double orignalWidth;
	double newWidth;

	double * originalPressure;
	double * newPressure;
	int pressureCount;
};

SizeUndoAction::SizeUndoAction(XojPage * page, Layer * layer, Redrawable * view) {
	this->page = page;
	this->layer = layer;
	this->view = view;
	this->data = NULL;
}

SizeUndoAction::~SizeUndoAction() {
	for (GList * l = this->data; l != NULL; l = l->next) {
		SizeUndoActionEntry * e = (SizeUndoActionEntry *) l->data;
		delete e;
	}

	g_list_free(this->data);
}

double * SizeUndoAction::getPressure(Stroke * s) {
	int count = s->getPointCount();
	double * data = new double[count];
	for (int i = 0; i < count; i++) {
		data[i] = s->getPoint(i).z;
	}

	return data;
}

void SizeUndoAction::addStroke(Stroke * s, double originalWidth, double newWidt, double * originalPressure,
		double * newPressure, int pressureCount) {
	this->data = g_list_append(this->data, new SizeUndoActionEntry(s, originalWidth, newWidt, originalPressure,
			newPressure, pressureCount));
}

bool SizeUndoAction::undo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	SizeUndoActionEntry * e = (SizeUndoActionEntry *) this->data->data;
	Range range(e->s->getX(), e->s->getY());

	for (GList * l = this->data; l != NULL; l = l->next) {
		SizeUndoActionEntry * e = (SizeUndoActionEntry *) l->data;
		e->s->setWidth(e->orignalWidth);
		e->s->setPressure(e->originalPressure);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX()+ e->s->getElementWidth(), e->s->getY()+ e->s->getElementHeight());
	}

	view->rerenderRange(range);

	return true;
}

bool SizeUndoAction::redo(Control * control) {
	if (this->data == NULL) {
		return true;
	}

	SizeUndoActionEntry * e = (SizeUndoActionEntry *) this->data->data;
	Range range(e->s->getX(), e->s->getY());

	for (GList * l = this->data; l != NULL; l = l->next) {
		SizeUndoActionEntry * e = (SizeUndoActionEntry *) l->data;
		e->s->setWidth(e->newWidth);
		e->s->setPressure(e->newPressure);

		range.addPoint(e->s->getX(), e->s->getY());
		range.addPoint(e->s->getX()+ e->s->getElementWidth(), e->s->getY()+ e->s->getElementHeight());
	}

	view->rerenderRange(range);

	return true;
}

String SizeUndoAction::getText() {
	return _("Change stroke width");
}
