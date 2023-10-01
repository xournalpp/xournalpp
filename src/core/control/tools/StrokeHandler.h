/*
 * Xournal++
 *
 * Handles input of strokes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

#include <gdk/gdk.h>  // for GdkEventKey

#include "model/PageRef.h"  // for PageRef
#include "model/Point.h"    // for Point

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

#define DEBUG_FPS
#ifdef DEBUG_FPS
#include <chrono>
#endif

class Control;
class Layer;
class PositionInputData;
class Stroke;

class Path;
class Spline;
class SplineSegment;
class PiecewiseLinearPath;

namespace SplineApproximator {
class Live;
};

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class OverlayView;
class Repaintable;
class StrokeToolView;
class StrokeToolLiveApproximationView;
};  // namespace xoj::view

namespace StrokeStabilizer {
class Base;
class Active;
class Deadzone;
class Inertia;
}  // namespace StrokeStabilizer

/**
 * @brief The stroke handler draws a stroke on a XojPageView
 *
 * The stroke is drawn using a cairo_surface_t* as a mask:
 * As the pointer moves on the canvas single segments are
 * drawn opaquely on the initially transparent masking
 * surface. The surface is used to mask the stroke
 * when drawing it to the XojPageView
 */
class StrokeHandler: public InputHandler {
public:
    StrokeHandler(Control* control, const PageRef& page);
    ~StrokeHandler() override;

    void onSequenceCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos, double zoom) override;
    void onButtonReleaseEvent(const PositionInputData& pos, double zoom) override;
    void onButtonPressEvent(const PositionInputData& pos, double zoom) override;
    void onButtonDoublePressEvent(const PositionInputData& pos, double zoom) override;
    bool onKeyEvent(GdkEventKey* event) override;

    /**
     * @brief Add a straight line to the stroke (if the movement is valid).
     * The line may be subdivided into smaller segments if the pressure variation is too big.
     * @param point The endpoint of the added line
     */
    void paintTo(Point point);

    auto createView(xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView> override;

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolView>>& getViewPool() const;
    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolLiveApproximationView>>& getApproxViewPool()
            const;

    // struct LiveApproximationData {
    //     const SplineSegment& liveSegment;
    //     const std::vector<Point>* const liveSegmentPointCache;
    //     const Spline& spline;
    //     const std::vector<Point>* splinePointCache;
    // };

protected:
    /**
     * @brief Unconditionally add a segment to the stroke.
     * Warning: it does not set the width properly nor test if the motion is valid. Use paintTo instead.
     * @param point The endpoint of the added segment
     */
    inline void drawSegmentTo(const Point& point) {
#ifdef DEBUG_FPS
        // Debug info. Remove
        ptCnt++;
        if (secondPassed()) {
            g_message("Points: %4zu  Frames : %2zu", ptCnt, frameCnt);
            ptCnt = 0;
            frameCnt = 0;
        }  //
#endif
        (this->*drawEvent)(point);
    };

    void strokeRecognizerDetected(std::shared_ptr<Path> result, Layer* layer);

protected:
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid.
    SnapToGridInputHandler snappingHandler;

private:
    /**
     * @brief Pointer to the Stabilizer instance
     */
    std::unique_ptr<StrokeStabilizer::Base> stabilizer;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolView>> viewPool;
    std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolLiveApproximationView>> approxViewPool;

    bool hasPressure;


    bool splineLiveApproximation;
    std::shared_ptr<PiecewiseLinearPath> path;

    void (StrokeHandler::*drawEvent)(const Point&);
    void normalDraw(const Point& p);
    void normalDrawLiveApproximator(const Point& p);

    std::shared_ptr<Spline> approximatedSpline;
    std::unique_ptr<SplineApproximator::Live> liveApprox;

public:
    const Spline& getSpline() const;
    const SplineSegment& getLiveSegment() const;

private:
#ifdef DEBUG_FPS
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    bool secondPassed() {
        auto t2 = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(t2 - t1) >= std::chrono::seconds(1)) {
            t1 = t2;
            return true;
        }
        return false;
    }
    size_t frameCnt = 0;
    size_t ptCnt = 0;
#endif


    friend class StrokeStabilizer::Active;
    friend class StrokeStabilizer::Deadzone;
    friend class StrokeStabilizer::Inertia;

    static constexpr double MAX_WIDTH_VARIATION = 0.3;
};
