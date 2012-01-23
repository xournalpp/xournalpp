#include "ColorUndoAction.h"

#include "../model/Element.h"
#include "../model/PageRef.h"
#include "../gui/Redrawable.h"

class ColorUndoActionEntry {
public:
	ColorUndoActionEntry(Element * e, int oldColor, int newColor) {
		this->e = e;
		this->oldColor = oldColor;
		this->newColor = newColor;
	}

	Element * e;
	int oldColor;
	int newColor;
};

ColorUndoAction::ColorUndoAction(PageRef page, Layer * layer, Redrawable * view) : UndoAction("ColorUndoAction") {
	XOJ_INIT_TYPE(ColorUndoAction);

	this->page = page;
	this->layer = layer;
	this->view = view;
	this->data = NULL;
}

ColorUndoAction::~ColorUndoAction() {
	XOJ_CHECK_TYPE(ColorUndoAction);

	for (GList * l = this->data; l != NULL; l = l->next) {
		ColorUndoActionEntry * e = (ColorUndoActionEntry *) l->data;
		delete e;
	}

	g_list_free(this->data);
	this->data = NULL;

	XOJ_RELEASE_TYPE(ColorUndoAction);
}

void ColorUndoAction::addStroke(Element * e, int originalColor, double newColor) {
	XOJ_CHECK_TYPE(ColorUndoAction);

	this->data = g_list_append(this->data, new ColorUndoActionEntry(e, originalColor, newColor));
}

bool ColorUndoAction::undo(Control * control) {
	XOJ_CHECK_TYPE(ColorUndoAction);

	if (this->data == NULL) {
		return true;
	}

	ColorUndoActionEntry * e = (ColorUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		ColorUndoActionEntry * e = (ColorUndoActionEntry *) l->data;
		e->e->setColor(e->oldColor);

		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	this->view->rerenderArea(x1, y1, x2, y2);

	return true;
}

bool ColorUndoAction::redo(Control * control) {
	XOJ_CHECK_TYPE(ColorUndoAction);

	if (this->data == NULL) {
		return true;
	}

	ColorUndoActionEntry * e = (ColorUndoActionEntry *) this->data->data;
	double x1 = e->e->getX();
	double x2 = e->e->getX() + e->e->getElementWidth();
	double y1 = e->e->getY();
	double y2 = e->e->getY() + e->e->getElementHeight();

	for (GList * l = this->data; l != NULL; l = l->next) {
		ColorUndoActionEntry * e = (ColorUndoActionEntry *) l->data;
		e->e->setColor(e->newColor);

		x1 = MIN(x1, e->e->getX());
		x2 = MAX(x2, e->e->getX()+ e->e->getElementWidth());
		y1 = MIN(y1, e->e->getY());
		y2 = MAX(y2, e->e->getY()+ e->e->getElementHeight());
	}

	this->view->rerenderArea(x1, y1, x2, y2);

	return true;
}

String ColorUndoAction::getText() {
	XOJ_CHECK_TYPE(ColorUndoAction);

	return _("Change color");
}
