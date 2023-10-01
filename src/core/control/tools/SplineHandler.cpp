#include "SplineHandler.h"

#include <algorithm>  // for max, max_element
#include <cmath>      // for pow, M_PI, cos, sin
#include <cstddef>    // for size_t
#include <memory>     // for allocator_traits<>...
#include <optional>   // for optional
#include <utility>    // for move
#include <vector>     // for vector

#include <gdk/gdkkeysyms.h>  // for GDK_KEY_Escape

#include "control/Control.h"                       // for Control
#include "control/layer/LayerController.h"         // for LayerController
#include "control/tools/InputHandler.h"            // for InputHandler
#include "control/tools/SnapToGridInputHandler.h"  // for SnapToGridInputHan...
#include "control/zoom/ZoomControl.h"
#include "gui/XournalppCursor.h"                 // for XournalppCursor
#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "model/Document.h"                      // for Document
#include "model/Layer.h"                         // for Layer
#include "model/SplineSegment.h"                 // for SplineSegment
#include "model/Stroke.h"                        // for Stroke
#include "model/XojPage.h"                       // for XojPage
#include "model/path/Spline.h"                   // for Spline
#include "undo/InsertUndoAction.h"               // for InsertUndoAction
#include "undo/UndoRedoHandler.h"                // for UndoRedoHandler
#include "util/Assert.h"                         // for xoj_assert
#include "util/DispatchPool.h"
#include "view/overlays/SplineToolView.h"

SplineHandler::SplineHandler(Control* control, const PageRef& page):
        InputHandler(control, page),
        snappingHandler(control->getSettings()),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SplineToolView>>()) {
    this->control->getZoomControl()->addZoomListener(this);
    this->knotsAttractionRadius = KNOTS_ATTRACTION_RADIUS_IN_PIXELS / this->control->getZoomControl()->getZoom();
}

SplineHandler::~SplineHandler() { this->control->getZoomControl()->removeZoomListener(this); }

std::unique_ptr<xoj::view::OverlayView> SplineHandler::createView(xoj::view::Repaintable* parent) const {
    return std::make_unique<xoj::view::SplineToolView>(this, parent);
}

constexpr double SHIFT_AMOUNT = 1.0;
constexpr double ROTATE_AMOUNT = 5.0;
constexpr double SCALE_AMOUNT = 1.05;
constexpr double MAX_TANGENT_LENGTH = 2000.0;
constexpr double MIN_TANGENT_LENGTH = 1.0;

auto SplineHandler::onKeyEvent(GdkEventKey* event) -> bool {
    if (!stroke || ((event->type != GDK_KEY_PRESS) ==
                    (event->keyval != GDK_KEY_Escape))) {  // except for escape key only act on key
                                                           // press event, not on key release event
        return false;
    }

    xoj_assert(!this->knots.empty() && this->knots.size() == this->tangents.size());
    Range rg = this->computeLastSegmentRepaintRange();

    switch (event->keyval) {
        case GDK_KEY_Escape: {
            this->finalizeSpline();
            return true;
        }
        case GDK_KEY_BackSpace: {
            if (this->knots.size() == 1) {
                return true;
            }
            this->deleteLastKnotWithTangent();
            xoj_assert(!this->knots.empty() && this->knots.size() == this->tangents.size());
            const Point& p = this->knots.back();
            const Point& t = this->tangents.back();
            rg.addPoint(p.x - t.x, p.y - t.y);  // Ensure the tangent vector gets its color updated
            break;
        }
        case GDK_KEY_Right: {
            this->movePoint(SHIFT_AMOUNT, 0);
            rg = rg.unite(this->computeLastSegmentRepaintRange());
            break;
        }
        case GDK_KEY_Left: {
            this->movePoint(-SHIFT_AMOUNT, 0);
            rg = rg.unite(this->computeLastSegmentRepaintRange());
            break;
        }
        case GDK_KEY_Up: {
            this->movePoint(0, -SHIFT_AMOUNT);
            rg = rg.unite(this->computeLastSegmentRepaintRange());
            break;
        }
        case GDK_KEY_Down: {
            this->movePoint(0, SHIFT_AMOUNT);
            rg = rg.unite(this->computeLastSegmentRepaintRange());
            break;
        }
        case GDK_KEY_r:
        case GDK_KEY_R: {  // r like "rotate"
            double angle = (event->state & GDK_SHIFT_MASK) ? -ROTATE_AMOUNT : ROTATE_AMOUNT;
            double xOld = this->tangents.back().x;
            double yOld = this->tangents.back().y;
            double xNew = cos(angle * M_PI / 180) * xOld + sin(angle * M_PI / 180) * yOld;
            double yNew = -sin(angle * M_PI / 180) * xOld + cos(angle * M_PI / 180) * yOld;
            this->modifyLastTangent(Point(xNew, yNew));
            rg = rg.unite(this->computeLastSegmentRepaintRange());
            break;
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
            rg = rg.unite(this->computeLastSegmentRepaintRange());
            break;
        }
        default:
            return false;
    }

    this->viewPool->dispatch(xoj::view::SplineToolView::FLAG_DIRTY_REGION, rg);
    return true;
}

auto SplineHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) -> bool {
    if (!stroke) {
        return false;
    }

    xoj_assert(!this->knots.empty() && this->knots.size() == this->tangents.size());

    Range rg = this->computeLastSegmentRepaintRange();
    if (this->isButtonPressed) {
        if (this->inFirstKnotAttractionZone) {
            // The button was pressed within the attraction zone. Wait for unpress to confirm/deny spline finalization
            return true;
        }
        Point newTangent = Point(pos.x / zoom - this->currPoint.x, pos.y / zoom - this->currPoint.y);
        if (validMotion(newTangent, this->tangents.back())) {
            this->modifyLastTangent(newTangent);
        }
    } else {
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        bool nowInAttractionZone =
                this->buttonDownPoint.lineLengthTo(this->knots.front()) < this->knotsAttractionRadius;
        if (nowInAttractionZone) {
            if (this->inFirstKnotAttractionZone) {
                // No need to update anything while staying in the attraction zone
                return true;
            }
        } else {
            this->currPoint = snappingHandler.snap(this->buttonDownPoint, knots.back(), pos.isAltDown());
        }
        this->inFirstKnotAttractionZone = nowInAttractionZone;
    }
    rg = rg.unite(this->computeLastSegmentRepaintRange());

    this->viewPool->dispatch(xoj::view::SplineToolView::FLAG_DIRTY_REGION, rg);
    return true;
}

void SplineHandler::onSequenceCancelEvent() {
    //  Touch screen sequence changed from normal to swipe/zoom/scroll sequence
    isButtonPressed = false;
    if (!stroke) {
        return;
    }

    if (this->knots.size() <= 1) {
        this->clearTinySpline();
    } else {
        auto rg = this->computeLastSegmentRepaintRange();
        this->deleteLastKnotWithTangent();
        this->viewPool->dispatch(xoj::view::SplineToolView::FLAG_DIRTY_REGION, rg);
    }
}

void SplineHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    this->isButtonPressed = false;
    if (this->inFirstKnotAttractionZone) {
        // The click began in the first knot's attraction zone
        // Finalize the spline if we still are in this zone, cancel the sequence otherwise
        const Point p(pos.x / zoom, pos.y / zoom);
        double dist = p.lineLengthTo(this->knots.front());
        if (dist < this->knotsAttractionRadius) {
            finalizeSpline();
        } else {
            this->inFirstKnotAttractionZone = false;
            onSequenceCancelEvent();
        }
    }
}

void SplineHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    this->isButtonPressed = true;

    if (!stroke) {
        // This should only happen right after the SplineHandler's creation, before any views got attached
        xoj_assert(this->viewPool->empty());

        stroke = createStroke(this->control);
        xoj_assert(this->knots.empty() && this->tangents.empty());
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        this->currPoint = snappingHandler.snapToGrid(this->buttonDownPoint, pos.isAltDown());
        this->addKnot(this->currPoint);
    } else {
        xoj_assert(!this->knots.empty());
        this->buttonDownPoint = Point(pos.x / zoom, pos.y / zoom);
        this->currPoint = snappingHandler.snap(this->buttonDownPoint, knots.back(), pos.isAltDown());
        double dist = this->buttonDownPoint.lineLengthTo(this->knots.front());
        if (dist < this->knotsAttractionRadius) {  // now the spline is closed and finalized
            this->addKnotWithTangent(this->knots.front(), this->tangents.front());
            this->inFirstKnotAttractionZone = true;
            auto rg = this->computeLastSegmentRepaintRange();
            this->viewPool->dispatch(xoj::view::SplineToolView::FLAG_DIRTY_REGION, rg);
        } else if (validMotion(currPoint, this->knots.back())) {
            this->addKnot(this->currPoint);
            auto rg = this->computeLastSegmentRepaintRange();
            this->viewPool->dispatch(xoj::view::SplineToolView::FLAG_DIRTY_REGION, rg);
        }
    }
}

void SplineHandler::onButtonDoublePressEvent(const PositionInputData&, double) { finalizeSpline(); }

void SplineHandler::movePoint(double dx, double dy) {
    // move last non dynamically changing point
    if (!this->knots.empty()) {
        this->knots.back().x += dx;
        this->knots.back().y += dy;
    }
}

void SplineHandler::clearTinySpline() {
    auto rg = this->computeLastSegmentRepaintRange();
    // Clearing the knots ensures the view will not draw anything (thus the repainting will erase everything)
    this->knots.clear();
    this->tangents.clear();
    this->stroke.reset();
    // Repaints and deletes the views
    this->viewPool->dispatchAndClear(xoj::view::SplineToolView::FINALIZATION_REQUEST, rg);
}

void SplineHandler::finalizeSpline() {
    xoj_assert(this->stroke);

    auto optData = getData();
    xoj_assert(optData);
    auto& data = optData.value();

    if (data.knots.size() < 2) {  // This is not a valid spline
        clearTinySpline();
        return;
    }

    // Reserve the right size
    std::shared_ptr<Spline> spline = std::make_shared<Spline>(this->knots[0], this->knots.size() - 1);
    for (size_t i = 0; i < this->knots.size() - 1; i++) {
        Point cp1 = Point(this->knots[i].x + this->tangents[i].x, this->knots[i].y + this->tangents[i].y);
        Point cp2 =
                Point(this->knots[i + 1].x - this->tangents[i + 1].x, this->knots[i + 1].y - this->tangents[i + 1].y);
        spline->addCubicSegment(cp1, cp2, this->knots[i + 1]);
    }
    xoj_assert(spline->nbSegments() == this->knots.size() - 1);
    stroke->setPath(std::move(spline));

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    Document* doc = control->getDocument();
    doc->lock();
    layer->addElement(stroke.get());
    doc->unlock();

    auto rg = this->computeTotalRepaintRange(data, stroke->getWidth());
    this->viewPool->dispatchAndClear(xoj::view::SplineToolView::FINALIZATION_REQUEST, rg);

    // Wait until this finishes before releasing `stroke`, so that PageView::elementChanged does not needlessly rerender
    // the stroke
    this->page->fireElementChanged(stroke.get());
    stroke.release();

    control->getCursor()->updateCursor();
}

void SplineHandler::zoomChanged() {
    this->knotsAttractionRadius = KNOTS_ATTRACTION_RADIUS_IN_PIXELS / this->control->getZoomControl()->getZoom();
}

void SplineHandler::addKnot(const Point& p) { addKnotWithTangent(p, Point(0, 0)); }

void SplineHandler::addKnotWithTangent(const Point& p, const Point& t) {
    this->knots.push_back(p);
    this->tangents.push_back(t);
}

void SplineHandler::modifyLastTangent(const Point& t) {
    xoj_assert(!this->tangents.empty());
    this->tangents.back() = t;
}

void SplineHandler::deleteLastKnotWithTangent() {
    xoj_assert(this->knots.size() > 1 && this->knots.size() == this->tangents.size());
    this->knots.pop_back();
    this->tangents.pop_back();
}

auto SplineHandler::computeTotalRepaintRange(const Data& data, double strokeWidth) const -> Range {
    std::vector<double> xCoords = {};
    std::vector<double> yCoords = {};
    for (auto p: data.knots) {
        xCoords.push_back(p.x);
        yCoords.push_back(p.y);
    }
    for (size_t i = 0; i < data.knots.size(); i++) {
        xCoords.push_back(data.knots[i].x + data.tangents[i].x);
        xCoords.push_back(data.knots[i].x - data.tangents[i].x);
        yCoords.push_back(data.knots[i].y + data.tangents[i].y);
        yCoords.push_back(data.knots[i].y - data.tangents[i].y);
    }
    xCoords.push_back(data.currPoint.x);
    yCoords.push_back(data.currPoint.y);

    double minX = *std::min_element(xCoords.begin(), xCoords.end());
    double maxX = *std::max_element(xCoords.begin(), xCoords.end());
    double minY = *std::min_element(yCoords.begin(), yCoords.end());
    double maxY = *std::max_element(yCoords.begin(), yCoords.end());

    Range rg(minX, minY, maxX, maxY);
    rg.addPadding(std::max(data.knotsAttractionRadius, strokeWidth));  // Circles around the knots and the spline width
    return rg;
}

Range SplineHandler::computeLastSegmentRepaintRange() const {
    xoj_assert(!this->knots.empty() && this->knots.size() == this->tangents.size());

    Range rg(this->currPoint.x, this->currPoint.y);
    const Point& p = this->knots.back();
    const Point& t = this->tangents.back();
    rg.addPoint(p.x + t.x, p.y + t.y);
    rg.addPoint(p.x - t.x, p.y - t.y);
    if (auto n = this->knots.size(); n > 1) {
        const Point& q = this->knots[n - 2];
        const Point& s = this->tangents[n - 2];
        rg.addPoint(q.x + s.x, q.y + s.y);
        rg.addPoint(q.x, q.y);  // Enough for the last segment.
    }

    // Ensure the range contains the spline (with its width) and the knots' circles
    rg.addPadding(std::max(this->knotsAttractionRadius, this->stroke->getWidth()));

    if (const Point& q = this->knots.front(); this->inFirstKnotAttractionZone) {
        // Make sure the range contains the last spline segment in case the spline is closed.
        // The last segment has a width fixed in pixels. The appropriate padding to account for this width will be added
        // on the View side.
        const double r = this->knotsAttractionRadius;
        rg = rg.unite(Range(q.x - r, q.y - r, q.x + r, q.y + r));
        const Point& s = this->tangents.front();
        rg.addPoint(q.x - s.x, q.y - s.y);
    } else if (this->stroke->getFill() != -1) {
        // If the stroke is filled, we need to update the filling as well. Changes in the filling happen in the convex
        // hull of the last segment and the first knot, so adding the first knot to the range is enough at this point.
        rg.addPoint(q.x, q.y);
    }
    return rg;
}

auto SplineHandler::getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SplineToolView>>& {
    return viewPool;
}

auto SplineHandler::getData() const -> std::optional<Data> {
    if (this->knots.empty()) {
        return std::nullopt;
    }
    return Data{this->knots, this->tangents, this->currPoint, this->knotsAttractionRadius,
                this->inFirstKnotAttractionZone};
}
