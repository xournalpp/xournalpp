#include "StrokeHandler.h"

#include <cmath>
#include <memory>

#include <gdk/gdk.h>

#include "control/Control.h"
#include "control/layer/LayerController.h"
#include "control/settings/Settings.h"
#include "control/shaperecognizer/ShapeRecognizer.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"
#include "undo/InsertUndoAction.h"
#include "undo/RecognizerUndoAction.h"
#include "view/StrokeView.h"

#include "StrokeStabilizer.h"
#include "config-features.h"

using namespace xoj::util;

guint32 StrokeHandler::lastStrokeTime;  // persist for next stroke

StrokeHandler::StrokeHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page):
        InputHandler(xournal, redrawable, page),
        snappingHandler(xournal->getControl()->getSettings()),
        stabilizer(StrokeStabilizer::get(xournal->getControl()->getSettings())) {}

void StrokeHandler::draw(cairo_t* cr) {
    assert(stroke && stroke->getPointCount() > 0);

    auto setColorAndBlendMode = [stroke = this->stroke, cr]() {
        if (stroke->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
            if (auto fill = stroke->getFill(); fill != -1) {
                Util::cairo_set_source_rgbi(cr, stroke->getColor(), static_cast<double>(fill) / 255.0);
            } else {
                Util::cairo_set_source_rgbi(cr, stroke->getColor(), xoj::view::StrokeView::OPACITY_HIGHLIGHTER);
            }
            cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
        } else {
            Util::cairo_set_source_rgbi(cr, stroke->getColor());
            cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        }
    };

    if (this->mask) {
        setColorAndBlendMode();
        cairo_mask_surface(cr, mask->surf, 0, 0);
    } else {
        if (this->stroke->getPointCount() == 1) {
            // drawStroke does not handle single dots
            const Point& pt = this->stroke->getPoint(0);
            double width = this->stroke->getWidth() * (this->hasPressure ? pt.z : 1.0);
            setColorAndBlendMode();
            this->paintDot(cr, pt.x, pt.y, width);
        } else {
            strokeView->draw(xoj::view::Context::createDefault(cr));
        }
    }
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

void StrokeHandler::paintTo(const Point& point) {

    int pointCount = stroke->getPointCount();

    if (pointCount > 0) {
        Point endPoint = stroke->getPoint(pointCount - 1);
        double distance = point.lineLengthTo(endPoint);
        if (distance < PIXEL_MOTION_THRESHOLD) {  //(!validMotion(point, endPoint)) {
            if (pointCount == 1 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                // Nb: at this point, neither point.z nor endPoint.z has been multiplied by this->stroke->getWidth()
                this->stroke->setLastPressure(point.z);
                this->firstPointPressureChange = true;

                double width = this->stroke->getWidth() * point.z;
                if (mask) {
                    this->paintDot(mask->cr, endPoint.x, endPoint.y, width);
                }
                // Trigger a call to `draw`. If mask == nullopt, the `paintDot` is called in `draw`
                this->redrawable->repaintRect(endPoint.x - 0.5 * width, endPoint.y - 0.5 * width, width, width);
            }
            return;
        }
        if (this->hasPressure) {
            /**
             * Both device and tool are pressure sensitive
             */
            if (this->firstPointPressureChange) {
                // Avoid shrinking if we recorded a higher pressure event at the beginning of the stroke
                this->firstPointPressureChange = false;
                stroke->setLastPressure(std::max(endPoint.z, point.z) * stroke->getWidth());
            } else {
                if (const double widthDelta = (point.z - endPoint.z) * stroke->getWidth();
                    - widthDelta > MAX_WIDTH_VARIATION || widthDelta > MAX_WIDTH_VARIATION) {
                    /**
                     * If the width variation is to big, decompose into shorter segments.
                     * Those segments can not be shorter than PIXEL_MOTION_THRESHOLD
                     */
                    double nbSteps = std::min(std::ceil(std::abs(widthDelta) / MAX_WIDTH_VARIATION),
                                              std::floor(distance / PIXEL_MOTION_THRESHOLD));
                    double stepLength = 1.0 / nbSteps;
                    Point increment((point.x - endPoint.x) * stepLength, (point.y - endPoint.y) * stepLength,
                                    widthDelta * stepLength);
                    endPoint.z *= stroke->getWidth();
                    endPoint.z += increment.z;
                    stroke->setLastPressure(endPoint.z);

                    for (int i = 1; i < static_cast<int>(nbSteps); i++) {  // The last step is done below
                        endPoint.x += increment.x;
                        endPoint.y += increment.y;
                        endPoint.z += increment.z;
                        drawSegmentTo(endPoint);
                    }
                }
                stroke->setLastPressure(point.z * stroke->getWidth());
            }
        }
    }
    drawSegmentTo(point);
}

void StrokeHandler::drawSegmentTo(const Point& point) {

    stroke->addPoint(this->hasPressure ? point : Point(point.x, point.y));

    double width = stroke->getWidth();

    assert(stroke->getPointCount() >= 2);
    const Point& prevPoint(stroke->getPoint(stroke->getPointCount() - 2));

    Range rg(prevPoint.x, prevPoint.y);
    rg.addPoint(point.x, point.y);

    if (stroke->getFill() != -1) {
        /**
         * Add the first point to the redraw range, so that the filling is painted.
         * Note: the actual stroke painting will only happen in this->draw() which is called less often
         */
        const Point& firstPoint = stroke->getPointVector().front();
        rg.addPoint(firstPoint.x, firstPoint.y);
    } else if (mask) {
        Stroke lastSegment;

        lastSegment.addPoint(prevPoint);
        lastSegment.addPoint(point);
        lastSegment.setWidth(width);

        auto context = xoj::view::Context::createColorBlind(mask->cr);
        xoj::view::StrokeView sView(&lastSegment);
        sView.draw(context);
    }

    width = prevPoint.z != Point::NO_PRESSURE ? prevPoint.z : width;

    // Trigger a call to `draw`. If mask == nullopt, the stroke is drawn in `draw`
    this->redrawable->repaintRect(rg.getX() - 0.5 * width, rg.getY() - 0.5 * width, rg.getWidth() + width,
                                  rg.getHeight() + width);
}

void StrokeHandler::onMotionCancelEvent() {
    delete stroke;
    stroke = nullptr;
}

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos) {
    if (!stroke) {
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
    if (auto const& pv = stroke->getPointVector(); pv.size() == 1) {
        const Point& pt = pv.front();
        if (this->hasPressure) {
            // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
            this->stroke->setLastPressure(std::max(pt.z, pos.pressure) * this->stroke->getWidth());
        }
        stroke->addPoint(pt);
    }

    stroke->freeUnusedPointItems();

    control->getLayerController()->ensureLayerExists(page);

    Layer* layer = page->getSelectedLayer();

    UndoRedoHandler* undo = control->getUndoRedoHandler();

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke));

    ToolHandler* h = control->getToolHandler();

    if (h->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        ShapeRecognizer reco;

        Stroke* recognized = reco.recognizePatterns(stroke);

        if (recognized) {
            strokeRecognizerDetected(recognized, layer);

            // Full repaint is done anyway
            // So repaint don't need to be done here

            stroke = nullptr;
            return;
        }
    }

    layer->addElement(stroke);
    page->fireElementChanged(stroke);

    // Manually force the rendering of the stroke, if no motion event occurred between, that would rerender the page.
    if (stroke->getPointCount() == 2) {
        this->redrawable->rerenderElement(stroke);
    }

    stroke = nullptr;
}

void StrokeHandler::strokeRecognizerDetected(Stroke* recognized, Layer* layer) {
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

    UndoRedoHandler* undo = xournal->getControl()->getUndoRedoHandler();
    undo->addUndoAction(std::move(recognizerUndo));
    layer->addElement(snappedStroke);

    Range range(snappedStroke->getX(), snappedStroke->getY());
    range.addPoint(snappedStroke->getX() + snappedStroke->getElementWidth(),
                   snappedStroke->getY() + snappedStroke->getElementHeight());

    range.addPoint(stroke->getX(), stroke->getY());
    range.addPoint(stroke->getX() + stroke->getElementWidth(), stroke->getY() + stroke->getElementHeight());

    page->fireRangeChanged(range);
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos) {
    const double zoom = xournal->getZoom();

    if (!stroke) {
        this->buttonDownPoint.x = pos.x / zoom;
        this->buttonDownPoint.y = pos.y / zoom;

        createStroke(Point(this->buttonDownPoint.x, this->buttonDownPoint.y, pos.pressure));

        this->hasPressure = this->stroke->getToolType() == STROKE_TOOL_PEN && pos.pressure != Point::NO_PRESSURE;

        stabilizer->initialize(this, zoom, pos);
    }

    double width = this->hasPressure ? this->stroke->getWidth() * pos.pressure : this->stroke->getWidth();

    bool needAMask = this->stroke->getFill() == -1 && !stroke->getLineStyle().hasDashes();
    if (needAMask) {
        // Strokes that require a full redraw don't use a mask
        this->createMask();
        this->paintDot(mask->cr, this->buttonDownPoint.x, this->buttonDownPoint.y, width);
    } else {
        strokeView.emplace(stroke);
    }
    this->redrawable->repaintRect(this->buttonDownPoint.x - 0.5 * width, this->buttonDownPoint.y - 0.5 * width, width,
                                  width);

    this->startStrokeTime = pos.timestamp;
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData& pos) {
    // nothing to do
}

void StrokeHandler::paintDot(cairo_t* cr, const double x, const double y, const double width) const {
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, width);
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, x, y);
    cairo_stroke(cr);
}

StrokeHandler::Mask::Mask(int width, int height) {
    surf = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
    cr = cairo_create(surf);
    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
}

StrokeHandler::Mask::~Mask() noexcept {
    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

void StrokeHandler::createMask() {
    const double ratio = xournal->getZoom() * static_cast<double>(xournal->getDpiScaleFactor());

    std::unique_ptr<Rectangle<double>> visibleRect(xournal->getVisibleRect(redrawable));

    // We add a padding to limit graphical bugs when scrolling right after completing a stroke
    const double strokeWidth = this->stroke->getWidth();
    const int width = static_cast<int>(std::ceil((visibleRect->width + strokeWidth) * ratio));
    const int height = static_cast<int>(std::ceil((visibleRect->height + strokeWidth) * ratio));

    mask.emplace(width, height);

    cairo_surface_set_device_offset(mask->surf, (0.5 * strokeWidth - visibleRect->x) * ratio,
                                    (0.5 * strokeWidth - visibleRect->y) * ratio);
    cairo_surface_set_device_scale(mask->surf, ratio, ratio);
}
