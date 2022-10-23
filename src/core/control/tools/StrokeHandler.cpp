#include "StrokeHandler.h"

#include <algorithm>  // for max, min
#include <cassert>    // for assert
#include <cfloat>     // for DBL_EPSILON
#include <cmath>      // for ceil, pow, abs
#include <memory>     // for unique_ptr, mak...
#include <utility>    // for move
#include <vector>     // for vector

#include <gdk/gdk.h>  // for GdkEventKey

#include "control/Control.h"                          // for Control
#include "control/ToolEnums.h"                        // for DRAWING_TYPE_ST...
#include "control/ToolHandler.h"                      // for ToolHandler
#include "control/layer/LayerController.h"            // for LayerController
#include "control/settings/Settings.h"                // for Settings
#include "control/shaperecognizer/ShapeRecognizer.h"  // for ShapeRecognizer
#include "control/tools/InputHandler.h"               // for InputHandler::P...
#include "control/tools/SnapToGridInputHandler.h"     // for SnapToGridInput...
#include "control/zoom/ZoomControl.h"
#include "gui/MainWindow.h"
#include "gui/PageView.h"                        // for XojPageView
#include "gui/XournalView.h"                     // for XournalView
#include "gui/inputdevices/PositionInputData.h"  // for PositionInputData
#include "model/Layer.h"                         // for Layer
#include "model/LineStyle.h"                     // for LineStyle
#include "model/Stroke.h"                        // for Stroke, STROKE_...
#include "model/XojPage.h"                       // for XojPage
#include "undo/InsertUndoAction.h"               // for InsertUndoAction
#include "undo/RecognizerUndoAction.h"           // for RecognizerUndoA...
#include "undo/UndoRedoHandler.h"                // for UndoRedoHandler
#include "util/Color.h"                          // for cairo_set_sourc...
#include "util/Range.h"                          // for Range
#include "util/Rectangle.h"                      // for Rectangle, util
#include "view/StrokeView.h"                     // for StrokeView, Str...
#include "view/View.h"                           // for Context

#include "StrokeStabilizer.h"  // for Base, get

using namespace xoj::util;

guint32 StrokeHandler::lastStrokeTime;  // persist for next stroke

StrokeHandler::StrokeHandler(Control* control, XojPageView* pageView, const PageRef& page):
        InputHandler(control, page),
        snappingHandler(control->getSettings()),
        stabilizer(StrokeStabilizer::get(control->getSettings())),
        pageView(pageView) {}

void StrokeHandler::draw(cairo_t* cr) {
    assert(stroke && stroke->getPointCount() > 0);

    auto setColorAndBlendMode = [stroke = this->stroke.get(), cr]() {
        if (stroke->getToolType() == StrokeTool::HIGHLIGHTER) {
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
                this->pageView->repaintRect(endPoint.x - 0.5 * width, endPoint.y - 0.5 * width, width, width);
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
    this->pageView->repaintRect(rg.getX() - 0.5 * width, rg.getY() - 0.5 * width, rg.getWidth() + width,
                                rg.getHeight() + width);
}

void StrokeHandler::onSequenceCancelEvent() { stroke.reset(); }

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    if (!stroke) {
        return;
    }

    /**
     * The stabilizer may have added a gap between the end of the stroke and the input device
     * Fill this gap.
     */
    stabilizer->finalizeStroke();

    Settings* settings = control->getSettings();

    if (settings->getStrokeFilterEnabled())  // Note: For shape tools see BaseStrokeHandler which has a slightly
                                             // different version of this filter. See //!
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
            if (pos.timestamp - StrokeHandler::lastStrokeTime > strokeFilterSuccessiveTime) {
                // stroke not being added to layer... delete here but clear first!

                this->pageView->rerenderRect(stroke->getX(), stroke->getY(), stroke->getElementWidth(),
                                             stroke->getElementHeight());  // clear onMotionNotifyEvent drawing //!

                stroke.reset();
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

    undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, stroke.get()));

    ToolHandler* h = control->getToolHandler();

    if (h->getDrawingType() == DRAWING_TYPE_STROKE_RECOGNIZER) {
        ShapeRecognizer reco;

        Stroke* recognized = reco.recognizePatterns(stroke.get(), control->getSettings()->getStrokeRecognizerMinSize());

        if (recognized) {
            strokeRecognizerDetected(recognized, layer);

            // Full repaint is done anyway
            // So repaint don't need to be done here

            stroke.release();  // The stroke is now owned by the UndoRedoHandler (to undo the recognition)
            return;
        }
    }


    Stroke* s = stroke.release();
    layer->addElement(s);
    page->fireElementChanged(s);

    // Manually force the rendering of the stroke, if no motion event occurred between, that would rerender the page.
    if (s->getPointCount() == 2) {
        this->pageView->rerenderElement(s);
    }
}

void StrokeHandler::strokeRecognizerDetected(Stroke* recognized, Layer* layer) {
    recognized->setWidth(stroke->hasPressure() ? stroke->getAvgPressure() : stroke->getWidth());

    // snapping
    if (control->getSettings()->getSnapRecognizedShapesEnabled()) {
        Rectangle<double> oldSnappedBounds = recognized->getSnappedBounds();
        Point topLeft = Point(oldSnappedBounds.x, oldSnappedBounds.y);
        Point topLeftSnapped = snappingHandler.snapToGrid(topLeft, false);

        recognized->move(topLeftSnapped.x - topLeft.x, topLeftSnapped.y - topLeft.y);
        Rectangle<double> snappedBounds = recognized->getSnappedBounds();
        Point belowRight = Point(snappedBounds.x + snappedBounds.width, snappedBounds.y + snappedBounds.height);
        Point belowRightSnapped = snappingHandler.snapToGrid(belowRight, false);

        double fx = (std::abs(snappedBounds.width) > DBL_EPSILON) ?
                            (belowRightSnapped.x - topLeftSnapped.x) / snappedBounds.width :
                            1;
        double fy = (std::abs(snappedBounds.height) > DBL_EPSILON) ?
                            (belowRightSnapped.y - topLeftSnapped.y) / snappedBounds.height :
                            1;
        recognized->scale(topLeftSnapped.x, topLeftSnapped.y, fx, fy, 0, false);
    }

    auto recognizerUndo = std::make_unique<RecognizerUndoAction>(page, layer, stroke.get(), recognized);

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    undo->addUndoAction(std::move(recognizerUndo));
    layer->addElement(recognized);

    Range range(recognized->getX(), recognized->getY());
    range.addPoint(recognized->getX() + recognized->getElementWidth(),
                   recognized->getY() + recognized->getElementHeight());

    range.addPoint(stroke->getX(), stroke->getY());
    range.addPoint(stroke->getX() + stroke->getElementWidth(), stroke->getY() + stroke->getElementHeight());

    page->fireRangeChanged(range);
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    if (!stroke) {
        this->buttonDownPoint.x = pos.x / zoom;
        this->buttonDownPoint.y = pos.y / zoom;

        stroke = createStroke(this->control);

        this->hasPressure = this->stroke->getToolType().isPressureSensitive() && pos.pressure != Point::NO_PRESSURE;

        double p = this->hasPressure ? pos.pressure : Point::NO_PRESSURE;
        stroke->addPoint(Point(this->buttonDownPoint.x, this->buttonDownPoint.y, p));

        stabilizer->initialize(this, zoom, pos);
    }

    double width = this->hasPressure ? this->stroke->getWidth() * pos.pressure : this->stroke->getWidth();

    bool needAMask = this->stroke->getFill() == -1 && !stroke->getLineStyle().hasDashes();
    if (needAMask) {
        // Strokes that require a full redraw don't use a mask
        this->createMask();
        this->paintDot(mask->cr, this->buttonDownPoint.x, this->buttonDownPoint.y, width);
    } else {
        strokeView.emplace(stroke.get());
    }
    this->pageView->repaintRect(this->buttonDownPoint.x - 0.5 * width, this->buttonDownPoint.y - 0.5 * width, width,
                                width);

    this->startStrokeTime = pos.timestamp;
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData&, double) {
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
    // todo(bhennion) Make xournal-independent on splitting
    auto xournal = control->getWindow()->getXournal();
    const double ratio = control->getZoomControl()->getZoom() * static_cast<double>(xournal->getDpiScaleFactor());

    std::unique_ptr<Rectangle<double>> visibleRect(xournal->getVisibleRect(pageView));

    // We add a padding to limit graphical bugs when scrolling right after completing a stroke
    const double strokeWidth = this->stroke->getWidth();
    const int width = static_cast<int>(std::ceil((visibleRect->width + strokeWidth) * ratio));
    const int height = static_cast<int>(std::ceil((visibleRect->height + strokeWidth) * ratio));

    mask.emplace(width, height);

    cairo_surface_set_device_offset(mask->surf, std::round((0.5 * strokeWidth - visibleRect->x) * ratio),
                                    std::round((0.5 * strokeWidth - visibleRect->y) * ratio));
    cairo_surface_set_device_scale(mask->surf, ratio, ratio);
}
