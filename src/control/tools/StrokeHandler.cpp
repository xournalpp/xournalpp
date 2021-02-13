#include "StrokeHandler.h"

#include <cmath>
#include <memory>

#include <gdk/gdk.h>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "control/settings/Settings.h"
#include "control/shaperecognizer/ShapeRecognizerResult.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "model/PiecewiseLinearPath.h"
#include "undo/InsertUndoAction.h"
#include "undo/RecognizerUndoAction.h"

#include "StrokeStabilizer.h"
#include "config-features.h"

guint32 StrokeHandler::lastStrokeTime;  // persist for next stroke


StrokeHandler::StrokeHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        InputHandler(xournal, redrawable, page),
        snappingHandler(xournal->getControl()->getSettings()),
        stabilizer(StrokeStabilizer::get(xournal->getControl()->getSettings())) {}

StrokeHandler::~StrokeHandler() {
    destroySurface();
    delete reco;
    reco = nullptr;
}

void StrokeHandler::draw(cairo_t* cr) {
    if (!stroke) {
        return;
    }

    DocumentView::applyColor(cr, stroke);

    if (stroke->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
        cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
    } else {
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    }

    cairo_mask_surface(cr, surfMask, 0, 0);
}


auto StrokeHandler::onKeyEvent(GdkEventKey* event) -> bool { return false; }


auto StrokeHandler::onMotionNotifyEvent(const PositionInputData& pos) -> bool {
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

    Point endPoint = this->path->getLastKnot();
    double distance = point.lineLengthTo(endPoint);
    if (distance < PIXEL_MOTION_THRESHOLD) {  //(!validMotion(point, endPoint)) {
        return;
    }
    if (this->hasPressure) {
        /**
         * Both device and tool are pressure sensitive
         */
        point.z *= stroke->getWidth();
        if (const double widthDelta = point.z - endPoint.z, absWidthDelta = std::abs(widthDelta);
            absWidthDelta > MAX_WIDTH_VARIATION) {
            /**
             * If the width variation is to big, decompose into shorter segments.
             * Those segments can not be shorter than PIXEL_MOTION_THRESHOLD
             */
            double nbSteps = std::min(std::ceil(absWidthDelta / MAX_WIDTH_VARIATION),
                                      std::floor(distance / PIXEL_MOTION_THRESHOLD));
            double stepLength = 1.0 / nbSteps;
            Point increment((point.x - endPoint.x) * stepLength, (point.y - endPoint.y) * stepLength,
                            widthDelta * stepLength);
            endPoint.z *= stroke->getWidth();
            endPoint.z += increment.z;

            for (int i = 1; i < static_cast<int>(nbSteps); i++) {  // The last step is done below
                endPoint.x += increment.x;
                endPoint.y += increment.y;
                endPoint.z += increment.z;
                drawSegmentTo(endPoint);
            }
        }
    }
    drawSegmentTo(point);
}

void StrokeHandler::drawSegmentTo(const Point& point) {

    Point previousPoint(this->path->getLastKnot());

    this->path->addLineSegmentTo(this->hasPressure ? point : Point(point.x, point.y));
    this->stroke->unsetSizeCalculated();

    const double width = stroke->getWidth();

    if ((stroke->getFill() != -1 || stroke->getLineStyle().hasDashes()) &&
        !(stroke->getFill() != -1 && stroke->getToolType() == STROKE_TOOL_HIGHLIGHTER)) {
        // Clear surface

        // for debugging purposes
        // cairo_set_source_rgba(crMask, 1, 0, 0, 1);
        cairo_set_source_rgba(crMask, 0, 0, 0, 0);
        cairo_rectangle(crMask, 0, 0, cairo_image_surface_get_width(surfMask),
                        cairo_image_surface_get_height(surfMask));
        cairo_fill(crMask);

        view.drawStroke(crMask, stroke, 0, 1, true, true);
        this->redrawable->repaintRect(stroke->getX() - width, stroke->getY() - width,
                                      stroke->getElementWidth() + 2 * width, stroke->getElementHeight() + 2 * width);
    } else {
        Stroke lastSegment;
        lastSegment.setPath(std::make_shared<PiecewiseLinearPath>(previousPoint, point));
        lastSegment.setWidth(width);

        cairo_set_operator(crMask, CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(crMask, 1, 1, 1, 1);

        view.drawStroke(crMask, &lastSegment, 0, 1, false);
        this->redrawable->repaintRect(lastSegment.getX(), lastSegment.getY(), lastSegment.getElementWidth(),
                                      lastSegment.getElementHeight());
    }
}

void StrokeHandler::onMotionCancelEvent() {
    delete stroke;
    stroke = nullptr;
}

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    if (!this->stroke || !this->path || this->path->empty()) {
        return;
    }

    /**
     * The stabilizer may have added a gap between the end of the stroke and the input device
     * Fill this gap.
     */
    stabilizer->finalizeStroke();


    Control* control = xournal->getControl();
    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled())  // Note: For shape tools see BaseStrokeHandler which has a slightly
                                             // different version of this filter. See //!
    {
        int strokeFilterIgnoreTime = 0, strokeFilterSuccessiveTime = 0;
        double strokeFilterIgnoreLength = NAN;

        settings->getStrokeFilter(&strokeFilterIgnoreTime, &strokeFilterIgnoreLength, &strokeFilterSuccessiveTime);
        double dpmm = settings->getDisplayDpi() / 25.4;

        double zoom = xournal->getZoom();

        double lengthSqrd = (pow(((pos.x / zoom) - (this->buttonDownPoint.x)), 2) +
                             pow(((pos.y / zoom) - (this->buttonDownPoint.y)), 2)) *
                            pow(xournal->getZoom(), 2);

        if (lengthSqrd < pow((strokeFilterIgnoreLength * dpmm), 2) &&
            pos.timestamp - this->startStrokeTime < strokeFilterIgnoreTime) {
            if (pos.timestamp - StrokeHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // stroke not being added to layer... delete here but clear first!

                this->redrawable->rerenderRect(stroke->getX(), stroke->getY(), stroke->getElementWidth(),
                                               stroke->getElementHeight());  // clear onMotionNotifyEvent drawing //!

                delete stroke;
                stroke = nullptr;
                this->userTapped = true;

                StrokeHandler::lastStrokeTime = pos.timestamp;

                return;
            }
        }
        StrokeHandler::lastStrokeTime = pos.timestamp;
    }

    // Backward compatibility and also easier to handle for me;-)
    // I cannot draw a line with one point, to draw a visible line I need two points,
    // twice the same Point is also OK
    if (this->path->nbSegments() == 0) {
        this->path->addLineSegmentTo(this->path->getFirstKnot());
        // Todo: check if the following is the reason for a bug, that single points have no pressure:
        // No pressure sensitivity,
        this->path->clearPressure();
    }

    stroke->freeUnusedPointItems();

    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke));

    ToolHandler* h = control->getToolHandler();

    if (h->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        if (reco == nullptr) {
            reco = new ShapeRecognizer();
        }

        ShapeRecognizerResult* result = reco->recognizePatterns(stroke);

        if (result) {
            strokeRecognizerDetected(result, layer);

            // Full repaint is done anyway
            // So repaint don't need to be done here

            stroke = nullptr;
            return;
        }
    }

    //     stroke->PLFromPoints();

    /**
     * Approximate the stroke by a spline using Schneider's algorithm
     */
    stroke->splineFromPLPath();
    this->path.reset();

    // TODO: Remove this when not needed (e.g. with no pressure) once the file format supports splines.
    // Always needed for rendering stroke with pressure values
    //     stroke->pointsFromPath();

    // Add the element
    layer->addElement(stroke);

    // Redraw after the spline approximation
    this->redrawable->rerenderElement(stroke);

// #define PAINT_BOUNDING_BOX
#ifdef PAINT_BOUNDING_BOX
    layer = page->getLayers()->back();

    Rectangle<double> bb = stroke->getSpline().getBoundingBox();
    Stroke* bbStroke = new Stroke();
    bbStroke->applyStyleFrom(stroke);
    bbStroke->setColor(Color(0xffff0000));
    bbStroke->addPoint(Point(bb.x, bb.y));
    bbStroke->addPoint(Point(bb.x, bb.y + bb.height));
    bbStroke->addPoint(Point(bb.x + bb.width, bb.y + bb.height));
    bbStroke->addPoint(Point(bb.x + bb.width, bb.y));
    bbStroke->addPoint(Point(bb.x, bb.y));
    layer->addElement(bbStroke);
    this->redrawable->rerenderElement(bbStroke);

    for (auto&& seg: stroke->getSpline()) {
        bbStroke = new Stroke();
        bbStroke->applyStyleFrom(stroke);
        bbStroke->setColor(Color(0xff0000ff));
        bbStroke->setWidth(bbStroke->getWidth() / 2.0);
        bb = seg.getBoundingBox();
        bbStroke->deletePointsFrom(0);
        bbStroke->addPoint(Point(bb.x, bb.y));
        bbStroke->addPoint(Point(bb.x, bb.y + bb.height));
        bbStroke->addPoint(Point(bb.x + bb.width, bb.y + bb.height));
        bbStroke->addPoint(Point(bb.x + bb.width, bb.y));
        bbStroke->addPoint(Point(bb.x, bb.y));
        layer->addElement(bbStroke);
        this->redrawable->rerenderElement(bbStroke);
    }
    bbStroke = nullptr;
#endif


    stroke = nullptr;
}

void StrokeHandler::strokeRecognizerDetected(ShapeRecognizerResult* result, Layer* layer) {
    Stroke* recognized = result->getRecognized();
    recognized->setWidth(stroke->hasPressure() ? stroke->getAvgPressure() : stroke->getWidth());

    // snapping
    Stroke* snappedStroke = recognized->cloneStroke();
    if (xournal->getControl()->getSettings()->getSnapRecognizedShapesEnabled()) {
        Rectangle<double> oldSnappedBounds = recognized->getSnappedBounds();
        Point topLeft = Point(oldSnappedBounds.x, oldSnappedBounds.y);
        Point topLeftSnapped = snappingHandler.snapToGrid(topLeft, false);

        snappedStroke->move(topLeftSnapped.x - topLeft.x, topLeftSnapped.y - topLeft.y);
        Rectangle<double> snappedBounds = snappedStroke->getSnappedBounds();
        Point belowRight = Point(snappedBounds.x + snappedBounds.width, snappedBounds.y + snappedBounds.height);
        Point belowRightSnapped = snappingHandler.snapToGrid(belowRight, false);

        double fx = (std::abs(snappedBounds.width) > DBL_EPSILON) ?
                            (belowRightSnapped.x - topLeftSnapped.x) / snappedBounds.width :
                            1;
        double fy = (std::abs(snappedBounds.height) > DBL_EPSILON) ?
                            (belowRightSnapped.y - topLeftSnapped.y) / snappedBounds.height :
                            1;
        snappedStroke->scale(topLeftSnapped.x, topLeftSnapped.y, fx, fy, 0, false);
    }

    auto recognizerUndo = std::make_unique<RecognizerUndoAction>(page, layer, stroke, snappedStroke);
    auto& locRecUndo = *recognizerUndo;

    UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();
    undo->addUndoAction(std::move(recognizerUndo));
    layer->addElement(snappedStroke);

    Range range(snappedStroke->getX(), snappedStroke->getY());
    range.addPoint(snappedStroke->getX() + snappedStroke->getElementWidth(),
                   snappedStroke->getY() + snappedStroke->getElementHeight());

    range.addPoint(stroke->getX(), stroke->getY());
    range.addPoint(stroke->getX() + stroke->getElementWidth(), stroke->getY() + stroke->getElementHeight());

    for (Stroke* s: *result->getSources()) {
        layer->removeElement(s, false);

        locRecUndo.addSourceElement(s);

        range.addPoint(s->getX(), s->getY());
        range.addPoint(s->getX() + s->getElementWidth(), s->getY() + s->getElementHeight());
    }

    page->fireRangeChanged(range);

    // delete the result object, this is not needed anymore, the stroke are not deleted with this
    delete result;
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos) {
    destroySurface();

    double zoom = xournal->getZoom();
    PageRef page = redrawable->getPage();

    int dpiScaleFactor = xournal->getDpiScaleFactor();

    double width = page->getWidth() * zoom * dpiScaleFactor;
    double height = page->getHeight() * zoom * dpiScaleFactor;

    surfMask = cairo_image_surface_create(CAIRO_FORMAT_A8, (int)std::ceil(width), (int)std::ceil(height));

    crMask = cairo_create(surfMask);

    // for debugging purposes
    // cairo_set_source_rgba(crMask, 0, 0, 0, 1);
    cairo_set_source_rgba(crMask, 0, 0, 0, 0);
    cairo_rectangle(crMask, 0, 0, width, height);

    cairo_fill(crMask);

    cairo_scale(crMask, zoom * dpiScaleFactor, zoom * dpiScaleFactor);

    if (!stroke) {
        this->buttonDownPoint.x = pos.x / zoom;
        this->buttonDownPoint.y = pos.y / zoom;

        createStroke();
        double pressure = pos.pressure != Point::NO_PRESSURE ? pos.pressure * stroke->getWidth() : Point::NO_PRESSURE;
        this->path = std::make_shared<PiecewiseLinearPath>(
                Point(this->buttonDownPoint.x, this->buttonDownPoint.y, pressure));

        this->stroke->setPath(this->path);

        this->hasPressure = this->stroke->getToolType() == STROKE_TOOL_PEN && pos.pressure != Point::NO_PRESSURE;

        stabilizer->initialize(this, zoom, pos);
    }

    this->startStrokeTime = pos.timestamp;
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
    // nothing to do
}

void StrokeHandler::destroySurface() {
    if (surfMask || crMask) {
        cairo_destroy(crMask);
        cairo_surface_destroy(surfMask);
        surfMask = nullptr;
        crMask = nullptr;
    }
}

void StrokeHandler::resetShapeRecognizer() {
    if (reco) {
        delete reco;
        reco = nullptr;
    }
}
