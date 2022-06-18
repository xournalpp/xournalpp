#include "SetsquareView.h"

#include <algorithm>  // for max, min
#include <cmath>      // for isnan, sqrt, cos, sin, atan2
#include <utility>    // for move

#include <glib.h>  // for g_error, g_warning

#include "control/Control.h"                // for Control
#include "control/ToolHandler.h"            // for ToolHandler
#include "control/layer/LayerController.h"  // for LayerController
#include "gui/PageView.h"                   // for XojPageView
#include "gui/XournalView.h"                // for XournalView
#include "gui/XournalppCursor.h"            // for XournalppCursor
#include "model/Layer.h"                    // for Layer
#include "model/Point.h"                    // for Point
#include "model/Setsquare.h"                // for Setsquare
#include "model/Stroke.h"                   // for Stroke
#include "model/XojPage.h"                  // for XojPage
#include "undo/InsertUndoAction.h"          // for InsertUndoAction
#include "undo/UndoRedoHandler.h"           // for UndoRedoHandler
#include "util/Rectangle.h"                 // for Rectangle
#include "view/View.h"                      // for Context

#include "StrokeView.h"  // for StrokeView

using xoj::util::Rectangle;

SetsquareView::SetsquareView(XojPageView* view, std::unique_ptr<Setsquare>& s): view(view), s(std::move(s)) {}

void SetsquareView::paint(cairo_t* cr) {
    const auto zoom = view->getXournal()->getZoom();
    cairo_save(cr);
    cairo_scale(cr, zoom, zoom);
    this->s->paint(cr);
    this->drawTemporaryStroke(cr);
    cairo_restore(cr);
}

void SetsquareView::drawTemporaryStroke(cairo_t* cr) {
    if (stroke) {
        auto context = xoj::view::Context::createDefault(cr);
        xoj::view::StrokeView strokeView(stroke);
        strokeView.draw(context);
    }
}

void SetsquareView::move(double x, double y) { s->move(x, y); }

void SetsquareView::rotate(double da, double cx, double cy) {
    s->rotate(da);
    const auto tx = getTranslationX();
    const auto ty = getTranslationY();
    const auto offsetX = tx - cx;
    const auto offsetY = ty - cy;
    const auto mx = offsetX * cos(da) - offsetY * sin(da);
    const auto my = offsetX * sin(da) + offsetY * cos(da);
    s->move(cx + mx - tx, cy + my - ty);
}

void SetsquareView::scale(double f, double cx, double cy) {
    s->scale(f);
    const auto tx = getTranslationX();
    const auto ty = getTranslationY();
    const auto offsetX = tx - cx;
    const auto offsetY = ty - cy;
    const auto mx = offsetX * f;
    const auto my = offsetY * f;
    s->move(cx + mx - tx, cy + my - ty);
}

auto SetsquareView::getHeight() const -> double { return s->getHeight(); }

auto SetsquareView::getRotation() const -> double { return s->getRotation(); }

auto SetsquareView::getTranslationX() const -> double { return s->getTranslationX(); }

auto SetsquareView::getTranslationY() const -> double { return s->getTranslationY(); }

auto SetsquareView::getView() const -> XojPageView* { return view; }

auto SetsquareView::getPage() const -> PageRef { return view->getPage(); }

auto SetsquareView::posRelToSide(Leg leg, double x, double y) const -> utl::Point<double> {
    cairo_matrix_t matrix{};
    s->getMatrix(matrix);
    cairo_matrix_invert(&matrix);
    cairo_matrix_transform_point(&matrix, &x, &y);
    switch (leg) {
        case HYPOTENUSE:
            return utl::Point<double>(x, -y);
        case LEFT_LEG:
            return utl::Point<double>((y + x) / sqrt(2.), (y - x - getHeight()) / sqrt(2.));
        case RIGHT_LEG:
            return utl::Point<double>((y - x) / sqrt(2.), (y + x - getHeight()) / sqrt(2.));
        default:
            g_error("Invalid enum value: %d", leg);
    }
}

auto SetsquareView::isInsideSetsquare(double x, double y, double border) const -> bool {
    return posRelToSide(HYPOTENUSE, x, y).y < border && posRelToSide(LEFT_LEG, x, y).y < border &&
           posRelToSide(RIGHT_LEG, x, y).y < border;
}

auto SetsquareView::getPointForPos(double xCoord) const -> utl::Point<double> {
    cairo_matrix_t matrix{};
    double x = xCoord;
    double y = 0.0;
    s->getMatrix(matrix);
    cairo_matrix_transform_point(&matrix, &x, &y);

    return utl::Point<double>(x, y);
}

void SetsquareView::createStroke(double x) {
    if (!std::isnan(x)) {
        hypotenuseMax = x;
        hypotenuseMin = x;

        const auto p = this->getPointForPos(x);
        initializeStroke();
        stroke->addPoint(Point(p.x, p.y));
        stroke->addPoint(Point(p.x, p.y));  // doubled point

    } else {
        g_warning("No valid stroke from setsquare!");
    }
}

void SetsquareView::createRadius(double x, double y) {
    const auto p = posRelToSide(HYPOTENUSE, x, y);
    this->strokeAngle = std::atan2(p.y, p.x);
    initializeStroke();
    updateRadius(x, y);
}

void SetsquareView::updateStroke(double x) {
    hypotenuseMax = std::max(this->hypotenuseMax, x);
    hypotenuseMin = std::min(this->hypotenuseMin, x);
    stroke->deletePointsFrom(0);
    const auto p1 = getPointForPos(hypotenuseMin);
    const auto p2 = getPointForPos(hypotenuseMax);

    stroke->addPoint(Point(p1.x, p1.y));
    stroke->addPoint(Point(p2.x, p2.y));
    const Rectangle<double> rect{stroke->getX(), stroke->getY(), stroke->getElementWidth(), stroke->getElementHeight()};
    this->getView()->rerenderRect(rect.x, rect.y, rect.width, rect.height);
}

void SetsquareView::updateRadius(double x, double y) {
    stroke->deletePointsFrom(0);
    const auto c = getPointForPos(0);
    stroke->addPoint(Point(c.x, c.y));

    const auto p = posRelToSide(HYPOTENUSE, x, y);
    const auto rad = std::hypot(p.x, p.y);

    if (rad >= s->getRadius()) {
        this->strokeAngle = std::atan2(p.y, p.x);
        stroke->addPoint(Point(x, y));
    } else {
        cairo_matrix_t matrix{};
        auto qx = rad * std::cos(this->strokeAngle);
        auto qy = -rad * std::sin(this->strokeAngle);
        s->getMatrix(matrix);
        cairo_matrix_transform_point(&matrix, &qx, &qy);

        stroke->addPoint(Point(qx, qy));
    }

    const Rectangle<double> rect{stroke->getX(), stroke->getY(), stroke->getElementWidth(), stroke->getElementHeight()};
    this->getView()->rerenderRect(rect.x, rect.y, rect.width, rect.height);
}

void SetsquareView::finalizeStroke() {
    hypotenuseMax = NAN;
    hypotenuseMin = NAN;
    addStrokeToLayer();
}

void SetsquareView::finalizeRadius() {
    strokeAngle = NAN;
    addStrokeToLayer();
}

auto SetsquareView::existsStroke() -> bool { return !std::isnan(hypotenuseMax) && !std::isnan(hypotenuseMin); }

auto SetsquareView::existsRadius() -> bool { return !std::isnan(strokeAngle); }

void SetsquareView::addStrokeToLayer() {
    const auto xournal = this->getView()->getXournal();
    const auto control = xournal->getControl();
    const auto page = getPage();
    control->getLayerController()->ensureLayerExists(page);
    const auto layer = page->getSelectedLayer();

    const auto undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke));

    layer->addElement(stroke);

    const Rectangle<double> rect{stroke->getX(), stroke->getY(), stroke->getElementWidth(), stroke->getElementHeight()};
    this->getView()->rerenderRect(rect.x, rect.y, rect.width, rect.height);
    stroke = nullptr;
    xournal->getCursor()->updateCursor();
}

void SetsquareView::initializeStroke() {
    const auto control = this->getView()->getXournal()->getControl();
    const auto h = control->getToolHandler();
    stroke = new Stroke();
    stroke->setWidth(h->getThickness());
    stroke->setColor(h->getColor());
    stroke->setFill(h->getFill());
    stroke->setLineStyle(h->getLineStyle());
}
