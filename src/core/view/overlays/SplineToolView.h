/*
 * Xournal++
 *
 * View active spline tool
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>

#include "control/zoom/ZoomListener.h"
#include "util/Color.h"
#include "util/DispatchPool.h"

#include "BaseShapeOrSplineToolView.h"

class SplineHandler;
class SplineHandlerData;
class OverlayBase;
class Range;
class ZoomControl;

namespace xoj::view {
class Repaintable;

class SplineToolView:
        public BaseShapeOrSplineToolView,
        public ZoomListener,
        public xoj::util::Listener<SplineToolView> {

public:
    SplineToolView(const SplineHandler* splineHandler, Repaintable* parent);
    virtual ~SplineToolView() noexcept;

    /**
     * @brief Draws the overlay to the given context
     */
    void draw(cairo_t* cr) const override;
    void drawWithoutDrawingAids(cairo_t* cr) const override;

    bool isViewOf(const OverlayBase* overlay) const override;

    /**
     * Zoom interface
     */
    void zoomChanged() override;

    /**
     * Listener interface
     */
    static constexpr struct FlagDirtyRegionRequest {
    } FLAG_DIRTY_REGION = {};
    void on(FlagDirtyRegionRequest, Range rg);

    /**
     * @brief Called before the corresponding SplineHandler's destruction
     * @param rg The bounding box of the entire spline + drawing aids, to be repainted
     */
    static constexpr struct FinalizationRequest {
    } FINALIZATION_REQUEST = {};
    void deleteOn(FinalizationRequest, Range rg);

private:
    void drawSpline(cairo_t* cr, const SplineHandlerData& data) const;

    const SplineHandler* splineHandler;

public:
    static constexpr double LINE_WIDTH_WITHOUT_ZOOM = 2.0;  // for tangent vectors & dynamic spline seg.
    static constexpr Color NODE_CIRCLE_COLOR = Colors::gray;
    static constexpr Color FIRST_NODE_CIRCLE_COLOR = Colors::red;
    static constexpr Color TANGENT_VECTOR_COLOR = Colors::lawngreen;
    static constexpr Color DYNAMIC_OBJECTS_COLOR = Colors::gray;
};
};  // namespace xoj::view
