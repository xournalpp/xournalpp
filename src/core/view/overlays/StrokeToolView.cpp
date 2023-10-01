#include "StrokeToolView.h"

#include <functional>
#include <memory>
#include <numeric>

#include "control/tools/StrokeHandler.h"
#include "model/LineStyle.h"
#include "model/Stroke.h"
#include "model/path/SegmentIteratable.h"
#include "util/Assert.h"
#include "util/Color.h"
#include "util/PairView.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "view/Repaintable.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

StrokeToolView::StrokeToolView(const StrokeHandler* strokeHandler, const Stroke& stroke, Repaintable* parent):
        BaseStrokeToolView(parent, stroke),
        strokeHandler(strokeHandler),
        pointBuffer(dynamic_cast<const PiecewiseLinearPath&>(stroke.getPath())) {
    this->registerToPool(strokeHandler->getViewPool());
    parent->flagDirtyRegion(Range(stroke.boundingRect()));
}

StrokeToolView::~StrokeToolView() noexcept { this->unregisterFromPool(); }

bool StrokeToolView::isViewOf(const OverlayBase* overlay) const { return overlay == this->strokeHandler; }

void StrokeToolView::draw(cairo_t* cr) const {

    PiecewiseLinearPath pts = this->flushBuffer();
    if (pts.empty()) {
        // The input sequence has probably been cancelled. This view should soon be deleted
        return;
    }
    // pts.front() is the last point we painted on the mask during the last iteration (see flushBuffer())

    if (!mask.isInitialized()) {
        // Initialize the mask on first call
        mask = createMask(cr);
        if (!mask.isInitialized()) {
            /*
             * The user might be drawing on a page that is not visible at all:
             * e.g. https://github.com/xournalpp/xournalpp/pull/4158#issuecomment-1385954494
             */
            return;
        }
    }

    xoj::util::CairoSaveGuard saveGuard(cr);
    cairo_set_operator(cr, this->cairoOp);

    this->drawFilling(cr, pts);  // Noop in the base class.

    Util::cairo_set_source_argb(cr, strokeColor);

    if (pts.nbSegments() >= 1) {
        // Draw the new segments on the mask
        if (pts.getFirstKnot().z == Point::NO_PRESSURE) {
            StrokeViewHelper::drawNoPressure(this->mask.get(), pts, this->strokeWidth, this->lineStyle,
                                             this->dashOffset);
            if (this->lineStyle.hasDashes()) {
                // Keep the offset up to date, so we do not have to redraw the entire stroke every time.
                auto segments = pts.segments();
                this->dashOffset =
                        std::transform_reduce(segments.begin(), segments.end(), this->dashOffset, std::plus<>(),
                                              [](auto& seg) { return seg.firstKnot.lineLengthTo(seg.secondKnot); });
            }
        } else {
            this->dashOffset = StrokeViewHelper::drawWithPressure(this->mask.get(), pts.getData(), this->lineStyle,
                                                                  this->dashOffset);
        }
    } else if (this->singleDot) {
        this->drawDot(this->mask.get(), pts.getLastKnot());
    }

    this->mask.blitTo(cr);
}

void StrokeToolView::on(StrokeToolView::AddPointRequest, const Point& p) {
    this->singleDot = false;
    xoj_assert(!this->pointBuffer.empty());  // front() is the last point we painted on the mask (see flushBuffer())
    Range repaintRange = getRepaintRange(this->pointBuffer.getLastKnot(), p);
    this->pointBuffer.addLineSegmentTo(p);
    this->parent->flagDirtyRegion(repaintRange);
}

void StrokeToolView::on(StrokeToolView::ThickenFirstPointRequest, double newWidth) {
    xoj_assert(newWidth > 0.0);
    xoj_assert(this->pointBuffer.getData().size() == 1);
    const Point& p = this->pointBuffer.getLastKnot();
    xoj_assert(p.z <= newWidth);  // Thicken means thicken
    this->pointBuffer.setFirstKnotPressure(newWidth);
    Range rg = Range(p.x, p.y);
    rg.addPadding(0.5 * newWidth);
    this->parent->flagDirtyRegion(rg);
}

void StrokeToolView::deleteOn(StrokeToolView::CancellationRequest, const Range& rg) {
    this->pointBuffer.clear();
    this->parent->drawAndDeleteToolView(this, rg);
}

void StrokeToolView::on(StrokeToolView::StrokeReplacementRequest, const Stroke& newStroke) {
    this->mask.wipe();
    this->pointBuffer = newStroke.getPointsToDraw();
    this->dashOffset = 0;
    this->strokeWidth = newStroke.getWidth();
    xoj_assert(this->strokeColor == strokeColorWithAlpha(newStroke));
    xoj_assert(this->lineStyle == newStroke.getLineStyle());
    xoj_assert(this->cairoOp ==
               (newStroke.getToolType() == StrokeTool::HIGHLIGHTER ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER));
}

void StrokeToolView::deleteOn(StrokeToolView::FinalizationRequest, const Range& rg) {
    this->parent->drawAndDeleteToolView(this, rg);
}

auto StrokeToolView::getRepaintRange(const Point& lastPoint, const Point& addedPoint) const -> Range {
    const double width = lastPoint.z == Point::NO_PRESSURE ? this->strokeWidth : lastPoint.z;
    Range rg(lastPoint.x, lastPoint.y);
    rg.addPoint(addedPoint.x, addedPoint.y);
    rg.addPadding(0.5 * width);
    return rg;
}

PiecewiseLinearPath StrokeToolView::flushBuffer() const {
    PiecewiseLinearPath pts;
    std::swap(this->pointBuffer, pts);
    if (!pts.empty()) {
        // Keep the last point in the buffer - to be used in the next iteration
        this->pointBuffer.setFirstPoint(pts.getLastKnot());
    }
    return pts;
}
