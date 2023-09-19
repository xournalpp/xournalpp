/*
 * Xournal++
 *
 * View active stroke tool - abstract base class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <vector>

#include <cairo.h>

#include "util/DispatchPool.h"
#include "util/Range.h"
#include "view/Mask.h"

#include "BaseStrokeToolView.h"

class StrokeHandler;
class Point;
class Range;
class Stroke;
class OverlayBase;
class Spline;
class SplineSegment;

namespace xoj::view {
class Repaintable;

class StrokeToolLiveApproximationView:
        public BaseStrokeToolView,
        public xoj::util::Listener<StrokeToolLiveApproximationView> {
public:
    StrokeToolLiveApproximationView(const StrokeHandler* strokeHandler, const Stroke& stroke, Repaintable* parent);
    virtual ~StrokeToolLiveApproximationView() noexcept;

    bool isViewOf(const OverlayBase* overlay) const override;

    void draw(cairo_t* cr) const override;

    /**
     * Listener interface
     */
    static constexpr struct UpdateLiveSegmentRequest {
    } UPDATE_LIVE_SEGMENT_REQUEST = {};
    void on(UpdateLiveSegmentRequest, const SplineSegment& liveSeg);

    static constexpr struct ThickenFirstPointRequest {
    } THICKEN_FIRST_POINT_REQUEST = {};
    void on(ThickenFirstPointRequest, double newPressure);

    static constexpr struct StrokeReplacementRequest {
    } STROKE_REPLACEMENT_REQUEST = {};
    void on(StrokeReplacementRequest, const Stroke& newStroke);

    static constexpr struct CancellationRequest {
    } CANCELLATION_REQUEST = {};
    void deleteOn(CancellationRequest, const Range& rg);

    /**
     * @brief Called before the corresponding StrokeHandler's destruction
     */
    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest, const Range& rg);

protected:
    // Nothing in the base class
    virtual void drawFilling(cairo_t*, const Spline& spline, const SplineSegment& liveSeg) const;

protected:
    const StrokeHandler* strokeHandler;

protected:
    bool singleDot = true;
    bool hasPressure;
    bool cancelled = false;
    Range liveRange;
    mutable size_t nbSegmentsOnMask = 0;

    /**
     * @brief offset for drawing dashes (if any)
     *      In effect, this stores the length of the path already drawn to the mask.
     */
    mutable double dashOffset = 0;

    /**
     * @brief Drawing mask.
     *
     * The stroke is drawn to the mask and the mask is then blitted wherever needed.
     * Upon calls to draw(), the buffer is flushed and the corresponding part of stroke is added to the mask.
     */
    mutable Mask mask;
    mutable Mask liveSegmentMask;
};
};  // namespace xoj::view
