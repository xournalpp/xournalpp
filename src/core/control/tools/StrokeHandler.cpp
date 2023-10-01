#include "StrokeHandler.h"

#include <algorithm>  // for max, min
#include <cmath>      // for ceil, pow, abs
#include <limits>     // for numeric_limits
#include <memory>     // for unique_ptr, mak...
#include <utility>    // for move
#include <vector>     // for vector

#include <gdk/gdk.h>  // for GdkEventKey

#include "control/Control.h"                                // for Control
#include "control/ToolEnums.h"                              // for DRAWING_TYPE_ST...
#include "control/ToolHandler.h"                            // for ToolHandler
#include "control/layer/LayerController.h"                  // for LayerController
#include "control/settings/Settings.h"                      // for Settings
#include "control/settings/SettingsEnums.h"                 // for EmptyLastPageAppendType
#include "control/shaperecognizer/ShapeRecognizer.h"        // for ShapeRecognizer
#include "control/tools/InputHandler.h"                     // for InputHandler::P...
#include "control/tools/SnapToGridInputHandler.h"           // for SnapToGridInput...
#include "gui/inputdevices/PositionInputData.h"             // for PositionInputData
#include "model/Document.h"                                 // for Document
#include "model/Layer.h"                                    // for Layer
#include "model/LineStyle.h"                                // for LineStyle
#include "model/Stroke.h"                                   // for Stroke, STROKE_...
#include "model/XojPage.h"                                  // for XojPage
#include "model/path/PiecewiseLinearPath.h"                 // for PiecewiseLinearPath
#include "splineapproximation/SplineApproximatorLive.h"     // for SplineApproximatorLive
#include "undo/InsertUndoAction.h"                          // for InsertUndoAction
#include "undo/RecognizerUndoAction.h"                      // for RecognizerUndoA...
#include "undo/UndoRedoHandler.h"                           // for UndoRedoHandler
#include "util/Assert.h"                                    // for xoj_assert
#include "util/DispatchPool.h"                              // for DispatchPool
#include "util/Range.h"                                     // for Range
#include "util/Rectangle.h"                                 // for Rectangle, util
#include "view/overlays/StrokeToolFilledHighlighterView.h"  // for StrokeToolFilledHighlighterView
#include "view/overlays/StrokeToolFilledView.h"             // for StrokeToolFilledView
#include "view/overlays/StrokeToolLiveApproximationView.h"  // for StrokeToolLiveApproximationView
#include "view/overlays/StrokeToolView.h"                   // for StrokeToolView

#include "StrokeStabilizer.h"  // for Base, get


StrokeHandler::StrokeHandler(Control* control, const PageRef& page):
        InputHandler(control, page),
        snappingHandler(control->getSettings()),
        stabilizer(StrokeStabilizer::get(control->getSettings())),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::StrokeToolView>>()),
        approxViewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::StrokeToolLiveApproximationView>>()) {}

StrokeHandler::~StrokeHandler() = default;

auto StrokeHandler::onKeyEvent(GdkEventKey* event) -> bool { return false; }

auto StrokeHandler::onMotionNotifyEvent(const PositionInputData& pos, double zoom) -> bool {
    if (!stroke) {
        return false;
    }

    if (pos.pressure == 0) {
        /**
         * Some devices emit a move event with pressure 0 when lifting the stylus tip
         * Ignore those events
         */
        return true;
    }

    stabilizer->processEvent(pos);
    return true;
}

void StrokeHandler::paintTo(Point point) {
    if (this->hasPressure && point.z > 0.0) {
        point.z *= this->stroke->getWidth();
    }

    if (!this->splineLiveApproximation) {
        const Point endPoint = this->path->getLastKnot();  // Make a copy as push to the vector might invalidate a ref.
        double distance = point.lineLengthTo(endPoint);
        if (distance < PIXEL_MOTION_THRESHOLD) {  //(!validMotion(point, endPoint)) {
            if (this->path->nbSegments() == 0 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                this->path->setFirstKnotPressure(point.z);
                this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, point.z);
            }
            return;
        }
        if (this->hasPressure && std::abs(point.z - endPoint.z) > MAX_WIDTH_VARIATION) {
            /**
             * Both device and tool are pressure sensitive.
             * If the width variation is to big, decompose into shorter segments.
             * Those segments can not be shorter than PIXEL_MOTION_THRESHOLD
             */
            MathVect3 increment(endPoint, point);
            double nbSteps = std::min(std::ceil(std::abs(increment.dz) / MAX_WIDTH_VARIATION),
                                      std::floor(distance / PIXEL_MOTION_THRESHOLD));
            double stepLength = 1.0 / nbSteps;
            increment *= stepLength;

            MathVect3 diffVector{0, 0, 0};
            for (int i = 1; i < static_cast<int>(nbSteps); i++) {
                // The last step is done at the end of paintTo()
                diffVector += increment;
                drawSegmentTo(diffVector.translatePoint(endPoint));
            }
        }
    } else {
        Point& endPoint = *this->liveApprox->P0;
        if (point.lineLengthTo(endPoint) < PIXEL_MOTION_THRESHOLD) {
            if (this->liveApprox->dataCount == 1 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                endPoint.z = point.z;
                this->approximatedSpline->setFirstKnotPressure(point.z);

                this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, point.z);
            }
            return;
        }
    }
    drawSegmentTo(point);
}
/*
void StrokeHandler::drawSegmentTo(const Point& point) {

    this->stroke->addPoint(this->hasPressure ? point : Point(point.x, point.y));
    this->viewPool->dispatch(xoj::view::StrokeToolView::ADD_POINT_REQUEST, this->stroke->getPointVector().back());
    return;
}*/

void StrokeHandler::onSequenceCancelEvent() {
    if (this->stroke) {
        this->viewPool->dispatchAndClear(xoj::view::StrokeToolView::CANCELLATION_REQUEST,
                                         Range(this->stroke->boundingRect()));
        this->approxViewPool->dispatchAndClear(xoj::view::StrokeToolLiveApproximationView::CANCELLATION_REQUEST,
                                               Range(this->stroke->boundingRect()));
        stroke.reset();
    }
}

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    if (!this->stroke ||
        ((!this->path || this->path->empty()) && (!this->approximatedSpline || this->approximatedSpline->empty()))) {
        g_warning("Empty stroke on button release!!");
        return;
    }

    /**
     * The stabilizer may have added a gap between the end of the stroke and the input device
     * Fill this gap.
     */
    stabilizer->finalizeStroke();

    bool enoughPoints = true;
    if (this->splineLiveApproximation) {
        enoughPoints = this->liveApprox->dataCount >= 3;
        if (enoughPoints) {
            // Try to approximate the last points
            bool lastFitSuccess = this->liveApprox->finalize();

            // Draw the last spline segment
            const SplineSegment& liveSegment = this->liveApprox->liveSegment;

            Range rg;
            if (hasPressure) {
                rg = liveSegment.getThickBoundingBox();
            } else {
                rg = liveSegment.getThinBoundingBox();
                rg.addPadding(0.5 * this->stroke->getWidth());
            }

            if (false) {  // mask) {
                Stroke liveSegmentStroke;
                std::shared_ptr<Spline> segs;
                if (lastFitSuccess) {
                    segs = std::make_shared<Spline>(liveSegment);
                } else {
                    segs = std::make_shared<Spline>(
                            this->liveApprox->getSpline().getSegment(this->liveApprox->getSpline().nbSegments() - 1));
                    segs->addCubicSegment(liveSegment);
                }
                liveSegmentStroke.setPath(segs);
                liveSegmentStroke.setWidth(this->stroke->getWidth());
                liveSegmentStroke.setPressureSensitive(this->hasPressure);

                // xoj::view::StrokeView sView(&liveSegmentStroke);
                // sView.draw(xoj::view::Context::createColorBlind(mask->cr));
            } else {
                if (this->stroke->getFill() != -1) {
                    /**
                     * Need to fill the area delimited by liveSegment and firstKnot
                     * Add the first knot to the rectangle
                     */
                    const Point& firstPoint = this->approximatedSpline->getFirstKnot();
                    rg.addPoint(firstPoint.x, firstPoint.y);
                }
            }
            // this->pageView->repaintRect(rect.x, rect.y, rect.width, rect.height);
        } else {
            // The stroke only has 1 or 2 points.
            // Either a degenerate line segment, or an actual line segment
            if (this->liveApprox->dataCount == 1) {
                Point& pt = *this->liveApprox->P0;
                if (this->hasPressure) {
                    // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
                    pt.z = std::max(pt.z, pos.pressure * this->stroke->getWidth());
                }
                this->stroke->setPath(std::make_shared<PiecewiseLinearPath>(pt, pt));
            } else {
                // 2 points
                this->stroke->setPath(std::make_shared<PiecewiseLinearPath>(this->liveApprox->liveSegment.firstKnot,
                                                                            this->liveApprox->liveSegment.secondKnot));
            }
        }
        this->stroke->clearPointCache();
        this->liveApprox->printStats();
    } else {
        enoughPoints = this->path->nbSegments() >= 1;
        if (!enoughPoints) {
            // We cannot draw a line with one point, to draw a visible line we need two points,
            // Twice the same Point is also OK
            Point pt = this->path->getFirstKnot();  // Make a copy so that path->addLineSegmentTo(pt) below is not UB
            if (this->hasPressure && pos.pressure * this->stroke->getWidth() > pt.z) {
                // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
                const double newPressure = pos.pressure * this->stroke->getWidth();
                this->path->setFirstKnotPressure(newPressure);
                this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, newPressure);
            }
            this->path->close();  // Copy the first knot to make a second point.
        }
    }

    stroke->freeUnusedPointItems();

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    Settings* settings = control->getSettings();
    if (settings->getEmptyLastPageAppend() == EmptyLastPageAppendType::OnDrawOfLastPage) {
        auto* doc = control->getDocument();
        doc->lock();
        auto pdfPageCount = doc->getPdfPageCount();
        doc->unlock();
        if (pdfPageCount == 0) {
            auto currentPage = control->getCurrentPageNo();
            doc->lock();
            auto lastPage = doc->getPageCount() - 1;
            doc->unlock();
            if (currentPage == lastPage) {
                control->insertNewPage(currentPage + 1, false);
            }
        }
    }

    ToolHandler* h = control->getToolHandler();
    if (!this->splineLiveApproximation && h->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        ShapeRecognizer reco(*this->path);

        std::shared_ptr<Path> result = reco.recognizePatterns(control->getSettings()->getStrokeRecognizerMinSize());

        if (result) {
            // strokeRecognizerDetected handles the repainting and the deletion of the views.
            strokeRecognizerDetected(result, layer);
            return;
        }
    }

    Range repaintRange;

    const bool postApproximation =
            enoughPoints && this->control->getSettings()->getSplineApproximatorType() == SplineApproximator::Type::POST;
    if (postApproximation) {
        /**
         * Approximate the stroke by a spline using Schneider's algorithm
         * Make sure we repaint everything
         */
        repaintRange = path->getThickBoundingBox(stroke->getWidth());
        stroke->splineFromPLPath();
        repaintRange = repaintRange.unite(stroke->getPath().getThickBoundingBox(stroke->getWidth()));
    }

    Document* doc = control->getDocument();
    doc->lock();
    layer->addElement(stroke.get());
    doc->unlock();

    // Blitt the stroke to the page's buffer and delete all views.
    this->viewPool->dispatchAndClear(xoj::view::StrokeToolView::FINALIZATION_REQUEST, repaintRange);
    this->approxViewPool->dispatchAndClear(xoj::view::StrokeToolLiveApproximationView::FINALIZATION_REQUEST,
                                           repaintRange);

    page->fireElementChanged(stroke.get());
    stroke.release();
}

void StrokeHandler::strokeRecognizerDetected(std::shared_ptr<Path> result, Layer* layer) {
    // snapping
    if (control->getSettings()->getSnapRecognizedShapesEnabled()) {
        Range oldSnappedBounds = result->getThinBoundingBox();
        Point topLeft = Point(oldSnappedBounds.minX, oldSnappedBounds.minY);
        Point topLeftSnapped = snappingHandler.snapToGrid(topLeft, false);

        result->move(topLeftSnapped.x - topLeft.x, topLeftSnapped.y - topLeft.y);

        const double w = oldSnappedBounds.getWidth();
        const double h = oldSnappedBounds.getHeight();
        Point belowRight = Point(topLeftSnapped.x + w, topLeftSnapped.y + h);
        Point belowRightSnapped = snappingHandler.snapToGrid(belowRight, false);

        double fx = (std::abs(w) > std::numeric_limits<double>::epsilon()) ?
                            (belowRightSnapped.x - topLeftSnapped.x) / w :
                            1;
        double fy = (std::abs(h) > std::numeric_limits<double>::epsilon()) ?
                            (belowRightSnapped.y - topLeftSnapped.y) / h :
                            1;
        result->scale(topLeftSnapped.x, topLeftSnapped.y, fx, fy, 0, false);
    }

    std::unique_ptr<Stroke> recognized = std::make_unique<Stroke>();
    recognized->setPath(result);

    recognized->applyStyleFrom(this->stroke.get());
    recognized->setWidth(this->stroke->hasPressure() ? this->path->getAveragePressure() : this->stroke->getWidth());

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::make_unique<RecognizerUndoAction>(page, layer, stroke.get(), recognized.get()));

    Document* doc = control->getDocument();
    doc->lock();
    layer->addElement(recognized.get());
    doc->unlock();

    Range range(recognized->getX(), recognized->getY());
    range.addPoint(recognized->getX() + recognized->getElementWidth(),
                   recognized->getY() + recognized->getElementHeight());

    range.addPoint(stroke->getX(), stroke->getY());
    range.addPoint(stroke->getX() + stroke->getElementWidth(), stroke->getY() + stroke->getElementHeight());

    stroke.release();  // The stroke is now owned by the UndoRedoHandler (to undo the recognition)

    this->viewPool->dispatch(xoj::view::StrokeToolView::STROKE_REPLACEMENT_REQUEST, *recognized);

    // Blitt the new stroke to the page's buffer, delete all the views and refresh the area (so the recognized stroke
    // gets displayed instead of the old one).
    this->viewPool->dispatchAndClear(xoj::view::StrokeToolView::FINALIZATION_REQUEST, range);

    stroke = std::move(recognized);  // To ensure PageView::elementChanged knows `recognized` is handler by *this
    page->fireElementChanged(stroke.get());
    stroke.release();  // The recognized stroke is really owned by the layer
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    xoj_assert(!stroke);

    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    this->stroke = createStroke(this->control);

    this->hasPressure = this->stroke->getToolType().isPressureSensitive() && pos.pressure != Point::NO_PRESSURE;
    this->stroke->setPressureSensitive(this->hasPressure);

    this->splineLiveApproximation =
            this->control->getSettings()->getSplineApproximatorType() == SplineApproximator::Type::LIVE;

    const double firstKnotWidth = this->hasPressure ? pos.pressure * stroke->getWidth() : Point::NO_PRESSURE;

    // Setup stroke path
    if (this->splineLiveApproximation) {
        this->approximatedSpline =
                std::make_shared<Spline>(Point(this->buttonDownPoint.x, this->buttonDownPoint.y, firstKnotWidth));
        this->liveApprox = std::make_unique<SplineApproximator::Live>(this->approximatedSpline);
        this->stroke->setPath(this->approximatedSpline);

        this->drawEvent = &StrokeHandler::normalDrawLiveApproximator;
    } else {
        this->path = std::make_shared<PiecewiseLinearPath>(
                Point(this->buttonDownPoint.x, this->buttonDownPoint.y, firstKnotWidth));
        this->stroke->setPath(this->path);
        this->drawEvent = &StrokeHandler::normalDraw;
    }

    stabilizer->initialize(this, zoom, pos);
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData&, double) {
    // nothing to do
}

auto StrokeHandler::createView(xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView> {
    xoj_assert(this->stroke);
    const Stroke& s = *this->stroke;
    if (splineLiveApproximation) {
        return std::make_unique<xoj::view::StrokeToolLiveApproximationView>(this, s, parent);
    }
    if (s.getFill() != -1) {
        if (s.getToolType() == StrokeTool::HIGHLIGHTER) {
            // Filled highlighter requires to wipe the mask entirely at every iteration
            // It has a dedicated view class.
            return std::make_unique<xoj::view::StrokeToolFilledHighlighterView>(this, s, parent);
        } else {
            return std::make_unique<xoj::view::StrokeToolFilledView>(this, s, parent);
        }
    } else {
        return std::make_unique<xoj::view::StrokeToolView>(this, s, parent);
    }
}

auto StrokeHandler::getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolView>>& {
    return viewPool;
}
auto StrokeHandler::getApproxViewPool() const
        -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolLiveApproximationView>>& {
    return approxViewPool;
}

const Spline& StrokeHandler::getSpline() const {
    xoj_assert(splineLiveApproximation && approximatedSpline);
    return *approximatedSpline;
}
const SplineSegment& StrokeHandler::getLiveSegment() const { return liveApprox->liveSegment; }

void StrokeHandler::normalDraw(const Point& p) {
    this->path->addLineSegmentTo(this->hasPressure ? p : Point(p.x, p.y));
    this->viewPool->dispatch(xoj::view::StrokeToolView::ADD_POINT_REQUEST, this->path->getLastKnot());
}

void StrokeHandler::normalDrawLiveApproximator(const Point& p) {
    const bool newDefinitiveSegment = !this->liveApprox->feedPoint(p);

    this->approxViewPool->dispatch(xoj::view::StrokeToolLiveApproximationView::UPDATE_LIVE_SEGMENT_REQUEST,
                                   this->liveApprox->liveSegment);

    // const auto getBBox = hasPressure ? &SplineSegment::getThickBoundingBox : &SplineSegment::getThinBoundingBox;
    // Range rg = (liveSegment.*getBBox)();
    //
    // if (newDefinitiveSegment) {
    //     // Fitting failed. Use the last cached segment and start a new live segment
    //     const SplineSegment& seg = this->liveApprox->lastDefinitiveSegment;
    //     this->liveSegmentStroke->setPath(std::make_shared<Spline>(seg));
    //     this->liveSegmentStroke->clearPointCache();
    //     /*
    //             xoj::view::StrokeView sView(this->liveSegmentStroke.get());
    //             sView.draw(xoj::view::Context::createColorBlind(mask->cr));
    //     */
    //     rg = rg.unite((seg.*getBBox)());
    // }
    //
    // if (!hasPressure) {
    //     rg.addPadding(0.5 * this->stroke->getWidth());
    // }

    //  this->pageView->repaintRect(rg.getX(), rg.getY(), rg.getWidth(), rg.getHeight());
}
#ifdef false
void StrokeHandler::fullRedrawLiveApproximator(const Point& p) {
    const bool newDefinitiveSegment = !this->liveApprox->feedPoint(p);

    const SplineSegment& liveSegment = this->liveApprox->liveSegment;

    const auto getBBox = hasPressure ? &SplineSegment::getThickBoundingBox : &SplineSegment::getThinBoundingBox;
    Range rg = (liveSegment.*getBBox)();

    if (newDefinitiveSegment) {
        const SplineSegment& seg = this->liveApprox->lastDefinitiveSegment;
        if (this->hasPressure) {
            this->stroke->resizePointCache(this->liveSegmentPointCacheBegin);
            this->stroke->addToPointCache(seg);
            this->liveSegmentPointCacheBegin = this->stroke->getCacheSize();
        }

        rg = rg.unite((seg.*getBBox)());
    }

    if (!hasPressure) {
        rg.addPadding(0.5 * this->stroke->getWidth());
    }

    if (this->stroke->getFill() != -1) {
        /**
         * Need to fill the area delimited by liveSegment and firstKnot
         */
        const Point& firstPoint = this->approximatedSpline->getFirstKnot();
        rg.addPoint(firstPoint.x, firstPoint.y);
    }

    this->pageView->repaintRect(rg.getX(), rg.getY(), rg.getWidth(), rg.getHeight());
}
#endif
