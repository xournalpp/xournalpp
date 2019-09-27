#include "VerticalToolHandler.h"
#include <util/cpp14memory.h>

#include "model/Layer.h"
#include "undo/UndoRedoHandler.h"
#include "util/GtkColorWrapper.h"
#include "view/DocumentView.h"

VerticalToolHandler::VerticalToolHandler(Redrawable* view, const PageRef& page, double y, double zoom)
 : view(view)
 , page(page)
 , layer(this->page->getSelectedLayer())
 , startY(y)
 , endY(y)
{
	for (Element* e: *this->layer->getElements())
	{
		if (e->getY() >= y)
		{
			this->elements.push_back(e);
		}
	}

	for (Element* e: this->elements)
	{
		this->layer->removeElement(e, false);

		this->jumpY = std::max(this->jumpY, e->getY() + e->getElementHeight());
	}

	this->jumpY = this->page->getHeight() - this->jumpY;

	this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, this->page->getWidth() * zoom,
	                                            (this->page->getHeight() - y) * zoom);

	cairo_t* cr = cairo_create(this->crBuffer);
	cairo_scale(cr, zoom, zoom);
	cairo_translate(cr, 0, -y);
	DocumentView v;
	v.drawSelection(cr, this);

	cairo_destroy(cr);

	view->rerenderPage();
}

VerticalToolHandler::~VerticalToolHandler()
{
	this->view = nullptr;

	if (this->crBuffer)
	{
		cairo_surface_destroy(this->crBuffer);
		this->crBuffer = nullptr;
	}
}

void VerticalToolHandler::paint(cairo_t* cr, GdkRectangle* rect, double zoom)
{
	GtkColorWrapper selectionColor = view->getSelectionColor();

	cairo_set_line_width(cr, 1);

	selectionColor.apply(cr);

	double y;
	double height;

	if (this->startY < this->endY)
	{
		y = this->startY;
		height = this->endY - this->startY;
	}
	else
	{
		y = this->endY;
		height = this->startY - this->endY;
	}

	cairo_rectangle(cr, 0, y * zoom, this->page->getWidth() * zoom, height * zoom);

	cairo_stroke_preserve(cr);
	selectionColor.applyWithAlpha(cr, 0.3);
	cairo_fill(cr);

	cairo_set_source_surface(cr, this->crBuffer, 0, this->endY * zoom);
	cairo_paint(cr);
}

void VerticalToolHandler::currentPos(double x, double y)
{
	if (this->endY == y)
	{
		return;
	}
	double y1 = std::min(this->endY, y);

	this->endY = y;

	this->view->repaintRect(0, y1, this->page->getWidth(), this->page->getHeight());
}

vector<Element*>* VerticalToolHandler::getElements()
{
	return &this->elements;
}

std::unique_ptr<MoveUndoAction> VerticalToolHandler::finalize()
{
	double dY = this->endY - this->startY;

	auto undo =
	        mem::make_unique<MoveUndoAction>(this->layer, this->page, &this->elements, 0, dY, this->layer, this->page);

	for (Element* e: this->elements)
	{
		e->move(0, dY);

		this->layer->addElement(e);
	}

	view->rerenderPage();

	return undo;
}
