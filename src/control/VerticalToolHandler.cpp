#include "VerticalToolHandler.h"
#include "../view/DocumentView.h"
#include "../control/UndoRedoHandler.h"

VerticalToolHandler::VerticalToolHandler(Redrawable * view, XojPage * page, double y, double zoom) {
	this->startY = y;
	this->endY = y;
	this->view = view;
	this->page = page;
	this->page->reference();
	this->layer = this->page->getSelectedLayer();
	this->elements = NULL;
	this->jumpY = 0;

	ListIterator<Element *> it = this->layer->elementIterator();
	while (it.hasNext()) {
		Element * e = it.next();
		if (e->getY() >= y) {
			this->elements = g_list_append(this->elements, e);
		}
	}

	for (GList * l = this->elements; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		this->layer->removeElement(e, false);

		this->jumpY = MAX(this->jumpY, e->getY() + e->getElementHeight());
	}

	this->jumpY = this->page->getHeight() - this->jumpY;

	crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->page->getWidth() * zoom, (this->page->getHeight()
			- y) * zoom);
	cairo_t * cr = cairo_create(crBuffer);
	cairo_scale(cr, zoom, zoom);
	cairo_translate(cr, 0, -y);
	DocumentView v;
	v.drawSelection(cr, this);

	cairo_destroy(cr);

	view->repaint();
}

VerticalToolHandler::~VerticalToolHandler() {
	this->page->unreference();
	this->page = NULL;
	this->view = NULL;

	if (this->crBuffer) {
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = NULL;
	}

	g_list_free(this->elements);
	this->elements = NULL;
}

void VerticalToolHandler::paint(cairo_t * cr, GdkEventExpose *event, double zoom) {
	GdkColor selectionColor = view->getSelectionColor();

	// set the line always the same size on display
	cairo_set_line_width(cr, 1 / zoom);

	cairo_set_source_rgb(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0);

	double y;
	double height;

	if (this->startY < this->endY) {
		y = this->startY;
		height = this->endY - this->startY;
	} else {
		y = this->endY;
		height = this->startY - this->endY;
	}

	cairo_rectangle(cr, 0, y, this->page->getWidth(), height);

	cairo_stroke_preserve(cr);
	cairo_set_source_rgba(cr, selectionColor.red / 65536.0, selectionColor.green / 65536.0, selectionColor.blue
			/ 65536.0, 0.3);
	cairo_fill(cr);

	cairo_set_source_surface(cr, crBuffer, 0, this->endY);
	cairo_paint(cr);
}

void VerticalToolHandler::currentPos(double x, double y) {
	if (this->endY == y) {
		return;
	}
	double y1 = MIN(this->endY, y);

	this->endY = y;

	view->redrawDocumentRegion(0, y1, this->page->getWidth(), this->page->getHeight());

	double dY = this->endY - this->startY;


	// TODO: we should move to a *new* page, but we should it do a bit more intelligent
	// than only move all elements...
	// But how?

//	printf("dY %lf / %lf\n", dY, this->jumpY);
//	if (this->jumpY + 10 < dY) {
//		printf("add page\n");
//	} else if (this->jumpY > dY) {
//		printf("remove page\n");
//	}
}

GList * VerticalToolHandler::getElements() {
	return this->elements;
}

MoveUndoAction * VerticalToolHandler::finnalize() {
	double dY = this->endY - this->startY;

	MoveUndoAction * undo = new MoveUndoAction(this->page, this);

	for (GList * l = this->elements; l != NULL; l = l->next) {
		Element * e = (Element *) l->data;
		e->move(0, dY);

		this->layer->addElement(e);
	}

	undo->finalize(this);

	view->repaint();

	return undo;
}

