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
#include "model/Element.h"
#include "model/Layer.h"                                    // for Layer
#include "model/LineStyle.h"                                // for LineStyle
#include "model/Stroke.h"                                   // for Stroke, STROKE_...
#include "model/XojPage.h"                                  // for XojPage
#include "undo/InsertUndoAction.h"                          // for InsertUndoAction
#include "undo/RecognizerUndoAction.h"                      // for RecognizerUndoA...
#include "undo/UndoRedoHandler.h"                           // for UndoRedoHandler
#include "util/Assert.h"                                    // for xoj_assert
#include "util/DispatchPool.h"                              // for DispatchPool
#include "util/Range.h"                                     // for Range
#include "util/Rectangle.h"                                 // for Rectangle, util
#include "view/overlays/StrokeToolFilledHighlighterView.h"  // for StrokeToolFilledHighlighterView
#include "view/overlays/StrokeToolFilledView.h"             // for StrokeToolFilledView
#include "view/overlays/StrokeToolView.h"                   // for StrokeToolView

#include "StrokeStabilizer.h"  // for Base, get

using xoj::util::Rectangle;

StrokeHandler::StrokeHandler(Control* control, const PageRef& page):
        InputHandler(control, page),
        snappingHandler(control->getSettings()),
        stabilizer(StrokeStabilizer::get(control->getSettings())),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::StrokeToolView>>()) {}

StrokeHandler::~StrokeHandler() = default;

auto StrokeHandler::onKeyPressEvent(const KeyEvent&) -> bool { return false; }
auto StrokeHandler::onKeyReleaseEvent(const KeyEvent&) -> bool { return false; }

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

    size_t pointCount = stroke->getPointCount();

    if (pointCount > 0) {
        Point endPoint = stroke->getPoint(pointCount - 1);
        double distance = point.lineLengthTo(endPoint);
        if (distance < PIXEL_MOTION_THRESHOLD) {  //(!validMotion(point, endPoint)) {
            if (pointCount == 1 && this->hasPressure && endPoint.z < point.z) {
                // Record the possible increase in pressure for the first point
                this->stroke->setLastPressure(point.z);
                this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, point.z);
            }
            return;
        }
        if (this->hasPressure) {
            /**
             * Both device and tool are pressure sensitive
             */
            if (const double widthDelta = point.z - endPoint.z;
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
                endPoint.z += increment.z;

                for (int i = 1; i < static_cast<int>(nbSteps); i++) {  // The last step is done below
                    endPoint.x += increment.x;
                    endPoint.y += increment.y;
                    endPoint.z += increment.z;
                    drawSegmentTo(endPoint);
                }
            }
        }
    }
    drawSegmentTo(point);
}

void StrokeHandler::drawSegmentTo(const Point& point) {

    this->stroke->addPoint(this->hasPressure ? point : Point(point.x, point.y));
    this->viewPool->dispatch(xoj::view::StrokeToolView::ADD_POINT_REQUEST, this->stroke->getPointVector().back());
    return;
}

void StrokeHandler::onSequenceCancelEvent() {
    if (this->stroke) {
        this->viewPool->dispatchAndClear(xoj::view::StrokeToolView::CANCELLATION_REQUEST,
                                         Range(this->stroke->boundingRect()));
        stroke.reset();
    }
}

void StrokeHandler::onButtonReleaseEvent(const PositionInputData& pos, double zoom) {
    if (!stroke) {
        return;
    }

    /**
     * The stabilizer may have added a gap between the end of the stroke and the input device
     * Fill this gap.
     */
    stabilizer->finalizeStroke();

    // Backward compatibility and also easier to handle for me;-)
    // I cannot draw a line with one point, to draw a visible line I need two points,
    // twice the same Point is also OK
    if (auto const& pv = stroke->getPointVector(); pv.size() == 1) {
        const Point pt = pv.front();  // Make a copy, otherwise stroke->addPoint(pt); in UB
        if (this->hasPressure) {
            // Pressure inference provides a pressure value to the last event. Most devices set this value to 0.
            const double newPressure = std::max(pt.z, pos.pressure * this->stroke->getWidth());
            this->stroke->setLastPressure(newPressure);
            this->viewPool->dispatch(xoj::view::StrokeToolView::THICKEN_FIRST_POINT_REQUEST, newPressure);
        }
        stroke->addPoint(pt);
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
    if (h->getDrawingType() == DRAWING_TYPE_SHAPE_RECOGNIZER) {
        ShapeRecognizer reco;

        auto recognized = reco.recognizePatterns(stroke.get(), control->getSettings()->getStrokeRecognizerMinSize());

        if (recognized) {
            // strokeRecognizerDetected handles the repainting and the deletion of the views.
            strokeRecognizerDetected(std::move(recognized), layer);
            return;
        }
    }

    auto ptr = stroke.get();
    Document* doc = control->getDocument();
    doc->lock();
    layer->addElement(std::move(stroke));
    doc->unlock();

    // Blitt the stroke to the page's buffer and delete all views.
    // Passing the empty Range() as no actual redrawing is necessary at this point
    this->viewPool->dispatchAndClear(xoj::view::StrokeToolView::FINALIZATION_REQUEST, Range());

    page->fireElementChanged(ptr);
}

void StrokeHandler::strokeRecognizerDetected(std::unique_ptr<Stroke> recognized, Layer* layer) {
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

        double fx = (std::abs(snappedBounds.width) > std::numeric_limits<double>::epsilon()) ?
                            (belowRightSnapped.x - topLeftSnapped.x) / snappedBounds.width :
                            1;
        double fy = (std::abs(snappedBounds.height) > std::numeric_limits<double>::epsilon()) ?
                            (belowRightSnapped.y - topLeftSnapped.y) / snappedBounds.height :
                            1;
        recognized->scale(topLeftSnapped.x, topLeftSnapped.y, fx, fy, 0, false);
    }

    UndoRedoHandler* undo = control->getUndoRedoHandler();
    auto recognizedPtr = recognized.get();
    auto strokePtr = stroke.get();
    undo->addUndoAction(std::make_unique<RecognizerUndoAction>(page, layer, std::move(stroke), recognizedPtr));

    Document* doc = control->getDocument();
    doc->lock();
    layer->addElement(std::move(recognized));
    doc->unlock();

    Range range(recognizedPtr->getX(), recognizedPtr->getY());
    range.addPoint(recognizedPtr->getX() + recognizedPtr->getElementWidth(),
                   recognizedPtr->getY() + recognizedPtr->getElementHeight());

    range.addPoint(strokePtr->getX(), strokePtr->getY());
    range.addPoint(strokePtr->getX() + strokePtr->getElementWidth(), strokePtr->getY() + strokePtr->getElementHeight());

    this->viewPool->dispatch(xoj::view::StrokeToolView::STROKE_REPLACEMENT_REQUEST, *recognizedPtr);

    // Blitt the new stroke to the page's buffer, delete all the views and refresh the area (so the recognized stroke
    // gets displayed instead of the old one).
    this->viewPool->dispatchAndClear(xoj::view::StrokeToolView::FINALIZATION_REQUEST, range);
    page->fireElementChanged(recognizedPtr);
}

void StrokeHandler::onButtonPressEvent(const PositionInputData& pos, double zoom) {
    xoj_assert(!stroke);

    this->buttonDownPoint.x = pos.x / zoom;
    this->buttonDownPoint.y = pos.y / zoom;

    stroke = createStroke(this->control);

    this->hasPressure = this->stroke->getToolType().isPressureSensitive() && pos.pressure != Point::NO_PRESSURE;

    const double width = this->hasPressure ? pos.pressure * stroke->getWidth() : Point::NO_PRESSURE;
    stroke->addPoint(Point(this->buttonDownPoint.x, this->buttonDownPoint.y, width));

    stabilizer->initialize(this, zoom, pos);
}

void StrokeHandler::onButtonDoublePressEvent(const PositionInputData&, double) {
    // nothing to do
}

auto StrokeHandler::createView(xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView> {
    xoj_assert(this->stroke);
    const Stroke& s = *this->stroke;
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
