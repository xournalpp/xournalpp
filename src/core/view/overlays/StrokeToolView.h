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

#include "model/path/PiecewiseLinearPath.h"
#include "util/DispatchPool.h"
#include "view/Mask.h"

#include "BaseStrokeToolView.h"

class StrokeHandler;
class Point;
class Range;
class Stroke;
class OverlayBase;

namespace xoj::view {
class Repaintable;

class StrokeToolView: public BaseStrokeToolView, public xoj::util::Listener<StrokeToolView> {
public:
    StrokeToolView(const StrokeHandler* strokeHandler, const Stroke& stroke, Repaintable* parent);
    virtual ~StrokeToolView() noexcept;

    bool isViewOf(const OverlayBase* overlay) const override;

    void draw(cairo_t* cr) const override;

    /**
     * Listener interface
     */
    static constexpr struct AddPointRequest {
    } ADD_POINT_REQUEST = {};
    virtual void on(AddPointRequest, const Point& p);

    static constexpr struct ThickenFirstPointRequest {
    } THICKEN_FIRST_POINT_REQUEST = {};
    void on(ThickenFirstPointRequest, double newPressure);

    static constexpr struct StrokeReplacementRequest {
    } STROKE_REPLACEMENT_REQUEST = {};
    virtual void on(StrokeReplacementRequest, const Stroke& newStroke);

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
    /**
     * @brief Compute the bounding box of the given segment, taking stroke width into account.
     */
    auto getRepaintRange(const Point& lastPoint, const Point& addedPoint) const -> Range;

    /**
     * @brief (Thread-safe) Flush the communication buffer and returns its content.
     */
    PiecewiseLinearPath flushBuffer() const;

    // Nothing in the base class
    virtual void drawFilling(cairo_t*, const PiecewiseLinearPath&) const {}

protected:
    const StrokeHandler* strokeHandler;

protected:
    bool singleDot = true;

    /**
     * @brief offset for drawing dashes (if any)
     *      In effect, this stores the length of the path already drawn to the mask.
     */
    mutable double dashOffset = 0;

    /**
     * @brief Controller/View communication buffer
     */
    mutable PiecewiseLinearPath pointBuffer;

    /**
     * @brief Drawing mask.
     *
     * The stroke is drawn to the mask and the mask is then blitted wherever needed.
     * Upon calls to draw(), the buffer is flushed and the corresponding part of stroke is added to the mask.
     */
    mutable Mask mask;
};
};  // namespace xoj::view
