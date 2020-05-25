#include "VerticalToolHandler.h"

#include <cmath>
#include <memory>

#include "model/Layer.h"
#include "undo/UndoRedoHandler.h"
#include "view/DocumentView.h"

VerticalToolHandler::VerticalToolHandler(Redrawable* view, const PageRef& page, Settings* settings, double y,
                                         double zoom):
        view(view), page(page), layer(this->page->getSelectedLayer()), snappingHandler(settings) {
    double ySnapped = snappingHandler.snapVertically(y, false);
    this->startY = ySnapped;
    this->endY = ySnapped;
    for (Element* e: *this->layer->getElements()) {
        if (e->getY() >= y) {
            this->elements.push_back(e);
        }
    }

    for (Element* e: this->elements) {
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

VerticalToolHandler::~VerticalToolHandler() {
    this->view = nullptr;

    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }
}

void VerticalToolHandler::paint(cairo_t* cr, GdkRectangle* rect, double zoom) {
    GdkRGBA selectionColor = view->getSelectionColor();

    cairo_set_line_width(cr, 1);

    gdk_cairo_set_source_rgba(cr, &selectionColor);

    double y = NAN;
    double height = NAN;

    if (this->startY < this->endY) {
        y = this->startY;
        height = this->endY - this->startY;
    } else {
        y = this->endY;
        height = this->startY - this->endY;
    }

    cairo_rectangle(cr, 0, y * zoom, this->page->getWidth() * zoom, height * zoom);

    cairo_stroke_preserve(cr);
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
    gdk_cairo_set_source_rgba(cr, &applied);
    cairo_fill(cr);

    cairo_set_source_surface(cr, this->crBuffer, 0, this->endY * zoom);
    cairo_paint(cr);
}

void VerticalToolHandler::currentPos(double x, double y) {
    double ySnapped = snappingHandler.snapVertically(y, false);
    if (this->endY == ySnapped) {
        return;
    }
    double y1 = std::min(this->endY, ySnapped);

    this->endY = ySnapped;

    this->view->repaintRect(0, y1, this->page->getWidth(), this->page->getHeight());
}

auto VerticalToolHandler::getElements() -> vector<Element*>* { return &this->elements; }

auto VerticalToolHandler::finalize() -> std::unique_ptr<MoveUndoAction> {
    double dY = this->endY - this->startY;

    auto undo =
            std::make_unique<MoveUndoAction>(this->layer, this->page, &this->elements, 0, dY, this->layer, this->page);

    for (Element* e: this->elements) {
        e->move(0, dY);

        this->layer->addElement(e);
    }

    view->rerenderPage();

    return undo;
}
