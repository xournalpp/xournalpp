#include "StrokeToolLiveApproximationView.h"

#include <functional>
#include <memory>
#include <numeric>

#include "control/tools/StrokeHandler.h"
#include "model/LineStyle.h"
#include "model/SplineSegment.h"
#include "model/Stroke.h"
#include "model/path/SegmentIteratable.h"
#include "model/path/Spline.h"
#include "util/Assert.h"
#include "util/Color.h"
#include "util/PairView.h"
#include "util/Range.h"
#include "util/Util.h"
#include "util/raii/CairoWrappers.h"  // for CairoSaveGuard
#include "view/Repaintable.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

StrokeToolLiveApproximationView::StrokeToolLiveApproximationView(const StrokeHandler* strokeHandler,
                                                                 const Stroke& stroke, Repaintable* parent):
        BaseStrokeToolView(parent, stroke), strokeHandler(strokeHandler), hasPressure(stroke.hasPressure()) {
    this->registerToPool(strokeHandler->getApproxViewPool());
    parent->flagDirtyRegion(Range(stroke.boundingRect()));
}

StrokeToolLiveApproximationView::~StrokeToolLiveApproximationView() noexcept { this->unregisterFromPool(); }

bool StrokeToolLiveApproximationView::isViewOf(const OverlayBase* overlay) const { return overlay == strokeHandler; }

void StrokeToolLiveApproximationView::drawFilling(cairo_t*, const Spline& spline, const SplineSegment& liveSeg) const {}

void StrokeToolLiveApproximationView::draw(cairo_t* cr) const {
    if (cancelled) {
        return;
    }

    if (!mask.isInitialized()) {
        // Initialize the mask on first call
        liveSegmentMask = createMask(cr);
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


    auto& spline = strokeHandler->getSpline();
    auto& liveSeg = strokeHandler->getLiveSegment();

    this->drawFilling(cr, spline, liveSeg);  // Noop in the base class.

    Util::cairo_set_source_argb(cr, strokeColor);


    if (!this->singleDot) {
        if (size_t nbSegs = spline.nbSegments(); nbSegs > nbSegmentsOnMask) {
            // Draw the new segments on the mask

            auto segments = spline.segments();
            auto firstNewSeg = std::next(segments.begin(), static_cast<ptrdiff_t>(nbSegmentsOnMask));

            if (!hasPressure) {
                cairo_set_line_width(this->mask.get(), strokeWidth);
                const auto& dashes = lineStyle.getDashes();
                Util::cairo_set_dash_from_vector(this->mask.get(), dashes, dashOffset);
                for (auto it = firstNewSeg; it != segments.end(); it++) {
                    it->toCairo(this->mask.get());
                }
                cairo_stroke(this->mask.get());
                if (this->lineStyle.hasDashes()) {
                    // Keep the offset up to date, so we do not have to redraw the entire stroke every time.
                    this->dashOffset = std::transform_reduce(firstNewSeg, segments.end(), this->dashOffset,
                                                             std::plus<>(), [](auto& seg) { return seg.length(); });
                }
            } else {
                for (auto it = firstNewSeg; it != segments.end(); it++) {
                    std::vector<Point> pts;
                    it->toPoints(pts);
                    this->dashOffset = StrokeViewHelper::drawWithPressure(this->mask.get(), pts, this->lineStyle,
                                                                          this->dashOffset);
                }
            }
            nbSegmentsOnMask = nbSegs;
        }
        // Draw the live segment onto its mask
        liveSegmentMask.wipe();
        if (!hasPressure) {
            cairo_set_line_width(this->liveSegmentMask.get(), strokeWidth);
            const auto& dashes = lineStyle.getDashes();
            Util::cairo_set_dash_from_vector(this->liveSegmentMask.get(), dashes, dashOffset);
            liveSeg.toCairo(this->liveSegmentMask.get());
            cairo_stroke(this->liveSegmentMask.get());
        } else {
            std::vector<Point> pts;
            liveSeg.toPoints(pts);
            StrokeViewHelper::drawWithPressure(this->liveSegmentMask.get(), pts, this->lineStyle, this->dashOffset);
        }
        // Blitt the main mask onto the liveSegmentMask: we can only blitt one mask onto `cr` (to avoid visual artefact
        // at the junction).
        this->mask.blitTo(this->liveSegmentMask.get());
    } else {
        this->drawDot(this->liveSegmentMask.get(), spline.getLastKnot());
    }

    this->liveSegmentMask.blitTo(cr);
}

void StrokeToolLiveApproximationView::on(UpdateLiveSegmentRequest, const SplineSegment& seg) {
    this->singleDot = false;
    Range repaintRange = liveRange;
    liveRange = seg.getThickBoundingBox();
    this->parent->flagDirtyRegion(repaintRange.unite(liveRange));
}

void StrokeToolLiveApproximationView::on(NewDefinitiveSegmentRequest, const SplineSegment& seg) {
    on(UPDATE_LIVE_SEGMENT_REQUEST, seg);  // TODO
}

void StrokeToolLiveApproximationView::on(StrokeToolLiveApproximationView::ThickenFirstPointRequest, double newWidth) {
    xoj_assert(hasPressure && newWidth > 0.0);
    auto& p = strokeHandler->getSpline().getFirstKnot();
    xoj_assert(p.z == newWidth);

    Range rg = Range(p.x, p.y);
    rg.addPadding(0.5 * newWidth);
    this->parent->flagDirtyRegion(rg);
}

void StrokeToolLiveApproximationView::deleteOn(StrokeToolLiveApproximationView::CancellationRequest, const Range& rg) {
    this->cancelled = true;
    this->parent->drawAndDeleteToolView(this, rg);
}

void StrokeToolLiveApproximationView::on(StrokeToolLiveApproximationView::StrokeReplacementRequest,
                                         const Stroke& newStroke) {
    this->mask.wipe();
    // TODO
    this->dashOffset = 0;
    this->strokeWidth = newStroke.getWidth();
    xoj_assert(this->strokeColor == strokeColorWithAlpha(newStroke));
    xoj_assert(this->lineStyle == newStroke.getLineStyle());
    xoj_assert(this->cairoOp ==
               (newStroke.getToolType() == StrokeTool::HIGHLIGHTER ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER));
}

void StrokeToolLiveApproximationView::deleteOn(StrokeToolLiveApproximationView::FinalizationRequest, const Range& rg) {
    this->parent->drawAndDeleteToolView(this, rg);
}
