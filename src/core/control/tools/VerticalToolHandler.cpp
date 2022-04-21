#include "VerticalToolHandler.h"

#include <cmath>
#include <memory>

#include <cairo.h>

#include "control/zoom/ZoomControl.h"
#include "model/Layer.h"
#include "undo/UndoRedoHandler.h"
#include "view/SelectionView.h"

VerticalToolHandler::VerticalToolHandler(Redrawable* view, const PageRef& page, Settings* settings, double y,
                                         bool initiallyReverse, ZoomControl* zoomControl, GdkWindow* window):
        window(window),
        view(view),
        page(page),
        layer(this->page->getSelectedLayer()),
        spacingSide(initiallyReverse ? Side::Above : Side::Below),
        zoom(0),
        zoomControl(zoomControl),
        snappingHandler(settings) {
    double ySnapped = snappingHandler.snapVertically(y, false);
    this->startY = ySnapped;
    this->endY = ySnapped;

    this->updateZoom(zoomControl->getZoom());

    this->adoptElements(this->spacingSide);

    view->rerenderPage();
}

VerticalToolHandler::~VerticalToolHandler() {
    this->view = nullptr;

    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }
}

void VerticalToolHandler::adoptElements(const Side side) {
    this->spacingSide = side;

    // Return current elements back to page
    for (Element* e: this->elements) { this->layer->addElement(e); }
    this->elements.clear();

    // Add new elements based on position
    for (Element* e: this->layer->getElements()) {
        if ((side == Side::Below && e->getY() >= this->startY) ||
            (side == Side::Above && e->getY() + e->getElementHeight() <= this->startY)) {
            this->elements.push_back(e);
        }
    }

    for (Element* e: this->elements) { this->layer->removeElement(e, false); }

    if (this->crBuffer) {
        redrawBuffer();
    }
}

void VerticalToolHandler::redrawBuffer() {
    g_assert_nonnull(this->crBuffer);

    cairo_t* cr = cairo_create(this->crBuffer);

    // Clear the buffer first
    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_restore(cr);

    // if below, render elements translated so that startY is 0.
    // if above, 0 is already the top of the page.
    if (this->spacingSide == Side::Below) {
        cairo_translate(cr, 0, -this->startY);
    } else {
        g_assert(this->spacingSide == Side::Above);
    }
    xoj::view::SelectionView v(this);
    v.draw(xoj::view::Context::createDefault(cr));

    cairo_destroy(cr);
}

void VerticalToolHandler::paint(cairo_t* cr) {
    GdkRGBA selectionColor = view->getSelectionColor();

    cairo_set_line_width(cr, 1);

    gdk_cairo_set_source_rgba(cr, &selectionColor);

    const double y = std::min(this->startY, this->endY);
    const double dy = this->endY - this->startY;

    cairo_rectangle(cr, 0, y, this->page->getWidth(), std::abs(dy));

    cairo_stroke_preserve(cr);
    auto applied = GdkRGBA{selectionColor.red, selectionColor.green, selectionColor.blue, 0.3};
    gdk_cairo_set_source_rgba(cr, &applied);
    cairo_fill(cr);


    const double elemY = (this->spacingSide == Side::Below ? this->endY : dy);
    cairo_set_source_surface(cr, this->crBuffer, 0, elemY);
    cairo_paint(cr);

#ifdef DEBUG_SHOW_PAINT_BOUNDS
    cairo_rectangle(cr, 0, elemY, cairo_image_surface_get_width(this->crBuffer),
                    cairo_image_surface_get_height(this->crBuffer));
    cairo_set_source_rgba(cr, 1.0, 0, 0, 0.3);
    cairo_fill(cr);
#endif
}

void VerticalToolHandler::currentPos(double x, double y) {
    double ySnapped = snappingHandler.snapVertically(y, false);
    if (this->endY == ySnapped) {
        return;
    }

    const double oldEnd = this->endY;
    this->endY = ySnapped;

    if (this->spacingSide == Side::Below) {
        this->view->rerenderRect(0, std::min(oldEnd, ySnapped), this->page->getWidth(), this->page->getHeight());
    } else {
        g_assert(this->spacingSide == Side::Above);
        this->view->rerenderRect(0, 0, this->page->getWidth(), std::max(oldEnd, ySnapped));
    }
}

bool VerticalToolHandler::onKeyPressEvent(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R) &&
        this->spacingSide == Side::Below) {
        this->adoptElements(Side::Above);
        this->view->rerenderPage();
        return true;
    }
    return false;
}

bool VerticalToolHandler::onKeyReleaseEvent(GdkEventKey* event) {
    if ((event->keyval == GDK_KEY_Control_L || event->keyval == GDK_KEY_Control_R) &&
        this->spacingSide == Side::Above) {
        this->adoptElements(Side::Below);
        this->view->rerenderPage();
        return true;
    }
    return false;
}

auto VerticalToolHandler::getElements() const -> const std::vector<Element*>& { return this->elements; }

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

void VerticalToolHandler::zoomChanged() {
    updateZoom(this->zoomControl->getZoom());
    redrawBuffer();
    this->view->rerenderPage();
}

void VerticalToolHandler::updateZoom(const double newZoom) {
    const auto oldZoom = this->zoom;
    this->zoom = newZoom;

    // The buffer only needs to be recreated if the zoom has increased.
    if (newZoom < oldZoom) {
        assert(this->crBuffer);
        cairo_surface_set_device_scale(this->crBuffer, newZoom, newZoom);
        return;
    }

    const int bufWidth = static_cast<int>(this->page->getWidth() * this->zoom);
    const int bufHeight = static_cast<int>(std::max(this->startY, this->page->getHeight() - this->startY) * this->zoom);

    if (this->crBuffer) {
        cairo_surface_destroy(this->crBuffer);
        this->crBuffer = nullptr;
    }

    if (this->window) {
        const int scale = gdk_window_get_scale_factor(this->window);
        this->crBuffer = gdk_window_create_similar_image_surface(this->window, CAIRO_FORMAT_ARGB32, bufWidth * scale,
                                                                 bufHeight * scale, scale);
    } else {
        this->crBuffer = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, bufWidth, bufHeight);
    }
    cairo_surface_set_device_scale(this->crBuffer, newZoom, newZoom);
};
