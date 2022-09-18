#include "SplineHandler.h"

#include <algorithm>  // for max, max_element
#include <cmath>      // for pow, M_PI, cos, sin
#include <cstddef>    // for size_t
#include <list>       // for list, operator!=
#include <memory>     // for allocator_traits<>...
#include <optional>   // for optional

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Escape

#include "control/Control.h"                       // for Control
#include "control/ToolEnums.h"                     // for DRAWING_TYPE_SPLINE
#include "control/ToolHandler.h"                   // for ToolHandler
#include "control/layer/LayerController.h"         // for LayerController
#include "control/settings/Settings.h"             // for Settings
#include "control/tools/InputHandler.h"            // for InputHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "control/zoom/ZoomControl.h"
#include "gui/LegacyRedrawable.h"                  // for LegacyRedrawable
#include "gui/XournalView.h"                       // for XournalView
#include "gui/XournalppCursor.h"                   // for XournalppCursor
#include "gui/inputdevices/PositionInputData.h"    // for PositionInputData
#include "model/Layer.h"                           // for Layer
#include "model/SplineSegment.h"                   // for SplineSegment
#include "model/Stroke.h"                          // for Stroke
#include "model/XojPage.h"                         // for XojPage
#include "undo/InsertUndoAction.h"                 // for InsertUndoAction
#include "undo/UndoRedoHandler.h"                  // for UndoRedoHandler
#include "view/StrokeView.h"                       // for StrokeView
#include "view/View.h"                             // for Context

using xoj::util::Rectangle;

guint32 SplineHandler::lastStrokeTime;  // persist for next stroke

SplineHandler::SplineHandler(Control* control, LegacyRedrawable* redrawable, const PageRef& page):
        InputHandler(control, page), snappingHandler(control->getSettings()), redrawable(redrawable) {}

SplineHandler::~SplineHandler() = default;

constexpr double RADIUS_WITHOUT_ZOOM = 10.0;
constexpr double LINE_WIDTH_WITHOUT_ZOOM = 2.0;

void SplineHandler::draw(cairo_t* cr) {
    if (!stroke || this->knots.empty()) {
        return;
    }

    if (control->getToolHandler()->getDrawingType() != DRAWING_TYPE_SPLINE) {
        g_warning("Drawing type is not spline any longer");
        this->finalizeSpline();
        this->knots.clear();
        this->tangents.clear();
        return;
    }

    double zoom = control->getZoomControl()->getZoom();
    double radius = RADIUS_WITHOUT_ZOOM / zoom;
    double lineWidth = LINE_WIDTH_WITHOUT_ZOOM / zoom;

    cairo_set_line_width(cr, lineWidth);
    const Point& firstKnot = this->knots.front();
    const Point& lastKnot = this->knots.back();
    const Point& firstTangent = this->tangents.front();
    const Point& lastTangent = this->tangents.back();
    double dist = this->buttonDownPoint.lineLengthTo(firstKnot);

    // draw circles around knot points
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);           // use gray color for all knots except first one
    for (auto p: knots) {                              // circle all knots, circle around first knot will be redrawn
        cairo_move_to(cr, p.x + radius, p.y);          // move to start point of circle arc;
        cairo_arc(cr, p.x, p.y, radius, 0, 2 * M_PI);  // draw circle
    }
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1, 0, 0);                             // use red color for first knot
    cairo_move_to(cr, firstKnot.x + radius, firstKnot.y);          // move to start point of circle arc;
    cairo_arc(cr, firstKnot.x, firstKnot.y, radius, 0, 2 * M_PI);  // draw circle
    if (dist<radius&& this->getKnotCount()> 1) {  // current point lies within the circle around the first knot
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }

    // draw dynamically changing segment
    cairo_set_source_rgb(cr, 0.3, 0.3, 0.3);  // use gray color
    const Point& cp1 = Point(lastKnot.x + lastTangent.x, lastKnot.y + lastTangent.y);
    const Point& cp2 = (dist<radius&& this->getKnotCount()> 1) ?
                               Point(firstKnot.x - firstTangent.x, firstKnot.y - firstTangent.y) :
                               this->currPoint;
    const Point& otherKnot = (dist<radius&& this->getKnotCount()> 1) ? this->buttonDownPoint : this->currPoint;
    SplineSegment changingSegment = SplineSegment(lastKnot, cp1, cp2, otherKnot);
    changingSegment.draw(cr);

    // draw dynamically changing tangent
    cairo_move_to(cr, lastKnot.x - lastTangent.x, lastKnot.y - lastTangent.y);
    cairo_line_to(cr, lastKnot.x + lastTangent.x, lastKnot.y + lastTangent.y);

    cairo_stroke(cr);


    // draw other tangents
    cairo_set_source_rgb(cr, 0, 1, 0);
    for (size_t i = 0; i < this->getKnotCount(); i++) {
        cairo_move_to(cr, this->knots[i].x - this->tangents[i].x,
                      this->knots[i].y - this->tangents[i].y);  // draw dynamically changing segment
        cairo_line_to(cr, this->knots[i].x + this->tangents[i].x, this->knots[i].y + this->tangents[i].y);
    }
    cairo_stroke(cr);

    // create stroke and draw spline
    this->updateStroke();
    if (this->getKnotCount() > 1) {
        auto context = xoj::view::Context::createDefault(cr);
        strokeView->draw(context);
    }
}

constexpr double SHIFT_AMOUNT = 1.0;
constexpr double ROTATE_AMOUNT = 5.0;
constexpr double SCALE_AMOUNT = 1.05;
constexpr double MAX_TANGENT_LENGTH = 2000.0;
constexpr double MIN_TANGENT_LENGTH = 1.0;

auto SplineHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (!stroke ||
        (event->type != GDK_KEY_PRESS && event->keyval != GDK_KEY_Escape)) {  // except for escape key only act on key
                                                                              // press event, not on key release event
        return false;
    }

    Rectangle<double> rect = this->computeRepaintRectangle();

    switch (event->keyval) {
        case GDK_KEY_Escape: {
            this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);
            this->finalizeSpline();
            return true;
        }
        case GDK_KEY_BackSpace: {
            if (!knots.empty()) {
                this->deleteLastKnotWithTangent();
                this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);
                return true;
            }
            break;
        }
        case GDK_KEY_Right: {
            this->movePoint(SHIFT_AMOUNT, 0);
            this->redrawable->repaintRect(rect.x, rect.y, rect.width + SHIFT_AMOUNT, rect.height);
            return true;
        }
        case GDK_KEY_Left: {
            this->movePoint(-SHIFT_AMOUNT, 0);
            this->redrawable->repaintRect(rect.x - SHIFT_AMOUNT, rect.y, rect.width, rect.height);
            return true;
        }
        case GDK_KEY_Up: {
            this->movePoint(0, -SHIFT_AMOUNT);
            this->redrawable->repaintRect(rect.x, rect.y - SHIFT_AMOUNT, rect.width, rect.height + SHIFT_AMOUNT);
            return true;
        }
        case GDK_KEY_Down: {
            this->movePoint(0, SHIFT_AMOUNT);
            this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height + SHIFT_AMOUNT);
            return true;
        }
        case GDK_KEY_r:
        case GDK_KEY_R: {  // r like "rotate"
            double angle = (event->state & GDK_SHIFT_MASK) ? -ROTATE_AMOUNT : ROTATE_AMOUNT;
            double xOld = this->tangents.back().x;
            double yOld = this->tangents.back().y;
            double xNew = cos(angle * M_PI / 180) * xOld + sin(angle * M_PI / 180) * yOld;
            double yNew = -sin(angle * M_PI / 180) * xOld + cos(angle * M_PI / 180) * yOld;
            this->modifyLastTangent(Point(xNew, yNew));
            this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);
            return true;
        }
        case GDK_KEY_s:
        case GDK_KEY_S: {  // s like "scale"
            double xOld = this->tangents.back().x;
            double yOld = this->tangents.back().y;
            double length = 2 * sqrt(pow(xOld, 2) + pow(yOld, 2));
            double factor = 1;
            if ((event->state & GDK_SHIFT_MASK) && length >= MIN_TANGENT_LENGTH) {
                factor = 1 / SCALE_AMOUNT;
            } else if (!(event->state & GDK_SHIFT_MASK) && length <= MAX_TANGENT_LENGTH) {
                factor = SCALE_AMOUNT;
            }
            double xNew = xOld * factor;
            double yNew = yOld * factor;
            this->modifyLastTangent(Point(xNew, yNew));
            this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);
            return true;
        }
    }
    return false;
}

auto SplineHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) -> bool {
    if (!stroke || this->knots.empty()) {
        return false;
    }

    Rectangle<double> rect = this->computeRepaintRectangle();
    if (this->isButtonPressed) {
        Point newTangent = Point(pos.x / zoom - this->currPoint.x, pos.y / zoom - this->currPoint.y);
        if (validMotion(newTangent, this->tangents.back())) {
            this->modifyLastTangent(newTangent);
        }
    } else {
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        this->currPoint = snappingHandler.snap(this->buttonDownPoint, knots.back(), pos.isAltDown());
    }

    rect.unite(this->computeRepaintRectangle());
    this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);

    return true;
}

void SplineHandler::onSequenceCancelEvent() { stroke.reset(); }

void SplineHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    isButtonPressed = false;

    if (!stroke) {
        return;
    }

    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled() && this->getKnotCount() < 2)  // Note: Mostly same as in BaseStrokeHandler
    {
        int strokeFilterIgnoreTime = 0, strokeFilterSuccessiveTime = 0;
        double strokeFilterIgnoreLength = NAN;

        settings->getStrokeFilter(&strokeFilterIgnoreTime, &strokeFilterIgnoreLength, &strokeFilterSuccessiveTime);
        double dpmm = settings->getDisplayDpi() / 25.4;

        double lengthSqrd = (pow(((pos.x / zoom) - (this->buttonDownPoint.x)), 2) +
                             pow(((pos.y / zoom) - (this->buttonDownPoint.y)), 2)) *
                            pow(zoom, 2);

        if (lengthSqrd < pow((strokeFilterIgnoreLength * dpmm), 2) &&
            pos.timestamp - this->startStrokeTime < strokeFilterIgnoreTime) {
            if (pos.timestamp - SplineHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // spline not being added to layer... delete here.
                this->finalizeSpline();
                this->knots.clear();
                this->tangents.clear();
                this->userTapped = true;

                SplineHandler::lastStrokeTime = pos.timestamp;

                control->getCursor()->updateCursor();

                return;
            }
        }
        SplineHandler::lastStrokeTime = pos.timestamp;
    }
}

void SplineHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    isButtonPressed = true;
    double radius = RADIUS_WITHOUT_ZOOM / zoom;
    this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
    this->currPoint = Point(pos.x / zoom, pos.y / zoom);

    if (!knots.empty()) {
        this->currPoint = snappingHandler.snap(this->currPoint, knots.back(), pos.isAltDown());
    } else {
        this->currPoint = snappingHandler.snapToGrid(this->currPoint, pos.isAltDown());
    }

    if (!stroke) {
        stroke = createStroke(this->control);
        stroke->addPoint(this->currPoint);
        strokeView.emplace(stroke.get());
        this->addKnot(this->currPoint);
        this->redrawable->rerenderRect(this->currPoint.x - radius, this->currPoint.y - radius, 2 * radius, 2 * radius);
    } else {
        double dist = this->buttonDownPoint.lineLengthTo(this->knots.front());
        if (dist < radius && !this->knots.empty()) {  // now the spline is closed and finalized
            this->addKnotWithTangent(this->knots.front(), this->tangents.front());
            this->finalizeSpline();
        } else if (validMotion(currPoint, this->knots.back())) {
            this->addKnot(this->currPoint);
            this->redrawable->rerenderRect(this->currPoint.x - radius, this->currPoint.y - radius, 2 * radius,
                                           2 * radius);
        }
    }
    this->startStrokeTime = pos.timestamp;
}

void SplineHandler::onButtonDoublePressEvent(const PositionInputData& pos, double zoom) { finalizeSpline(); }

void SplineHandler::movePoint(double dx, double dy) {
    // move last non dynamically changing point
    if (!this->knots.empty()) {
        this->knots.back().x += dx;
        this->knots.back().y += dy;
    }
}

void SplineHandler::finalizeSpline() {
    if (!stroke) {
        return;
    }

    if (this->getKnotCount() < 2) {  // This is not a valid spline
        Rectangle<double> rect = this->computeRepaintRectangle();
        stroke.reset();
        this->redrawable->repaintRect(rect.x, rect.y, rect.width, rect.height);

        return;
    }

    this->updateStroke();
    Rectangle<double> rect = this->computeRepaintRectangle();

    stroke->freeUnusedPointItems();
    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    layer->addElement(stroke.release());

    this->redrawable->rerenderRect(rect.x, rect.y, rect.width, rect.height);

    control->getCursor()->updateCursor();
}

void SplineHandler::addKnot(const Point& p) { addKnotWithTangent(p, Point(0, 0)); }

void SplineHandler::addKnotWithTangent(const Point& p, const Point& t) {
    this->knots.push_back(p);
    this->tangents.push_back(t);
}

void SplineHandler::modifyLastTangent(const Point& t) { this->tangents.back() = t; }

void SplineHandler::deleteLastKnotWithTangent() {
    if (this->getKnotCount() > 1) {
        this->knots.pop_back();
        this->tangents.pop_back();
    }
}

auto SplineHandler::getKnotCount() const -> size_t {
    if (this->knots.size() != this->tangents.size()) {
        g_warning("number of knots and tangents differ");
    }
    return this->knots.size();
}

void SplineHandler::updateStroke() {
    if (!stroke) {
        return;
    }
    // create spline segments
    std::vector<SplineSegment> segments = {};
    Point cp1, cp2;
    for (size_t i = 0; i < this->getKnotCount() - 1; i++) {
        cp1 = Point(this->knots[i].x + this->tangents[i].x, this->knots[i].y + this->tangents[i].y);
        cp2 = Point(this->knots[i + 1].x - this->tangents[i + 1].x, this->knots[i + 1].y - this->tangents[i + 1].y);
        segments.push_back(SplineSegment(this->knots[i], cp1, cp2, this->knots[i + 1]));
    }

    // convert collection of segments to stroke
    stroke->deletePointsFrom(0);
    for (auto s: segments) {
        for (auto p: s.toPointSequence()) { stroke->addPoint(p); }
    }
    if (!segments.empty()) {
        stroke->addPoint(segments.back().secondKnot);
    }
}

auto SplineHandler::computeRepaintRectangle() const -> Rectangle<double> {
    double zoom = control->getZoomControl()->getZoom();  // todo(bhennion) in splitting: remove zoom dependence
    double radius = RADIUS_WITHOUT_ZOOM / zoom;
    std::vector<double> xCoords = {};
    std::vector<double> yCoords = {};
    for (auto p: knots) {
        xCoords.push_back(p.x);
        yCoords.push_back(p.y);
    }
    for (size_t i = 0; i < this->getKnotCount(); i++) {
        xCoords.push_back(this->knots[i].x + this->tangents[i].x);
        xCoords.push_back(this->knots[i].x - this->tangents[i].x);
        yCoords.push_back(this->knots[i].y + this->tangents[i].y);
        yCoords.push_back(this->knots[i].y - this->tangents[i].y);
    }
    xCoords.push_back(this->currPoint.x);
    yCoords.push_back(this->currPoint.y);

    double minX = *std::min_element(xCoords.begin(), xCoords.end());
    double maxX = *std::max_element(xCoords.begin(), xCoords.end());
    double minY = *std::min_element(yCoords.begin(), yCoords.end());
    double maxY = *std::max_element(yCoords.begin(), yCoords.end());
    return Rectangle<double>(minX - radius, minY - radius, maxX - minX + 2 * radius, maxY - minY + 2 * radius);
}
