/*
 * Xournal++
 *
 * Handles input to draw a spline consisting of linear and cubic spline segments
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <vector>    // for vector

#include <gdk/gdk.h>  // for GdkEventKey

#include "control/zoom/ZoomListener.h"
#include "model/PageRef.h"   // for PageRef
#include "model/Point.h"     // for Point
#include "util/Range.h"      // for Range

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class PositionInputData;
class Control;

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class OverlayView;
class Repaintable;
class SplineToolView;
};  // namespace xoj::view

/**
 * @brief Helper structure for communication with the views
 */
struct SplineHandlerData {
    const std::vector<Point>& knots;
    const std::vector<Point>& tangents;
    const Point& currPoint;
    double knotsAttractionRadius;
    bool closedSpline;
};

/**
 * @brief A class to handle splines
 *
 * Drawing of a spline is started by a ButtonPressEvent. Every ButtonPressEvent gives a new knot.
 * Click-dragging will set the tangents. After a ButtonReleaseEvent the spline segment is dynamically
 * drawn and finished after the next ButtonPressEvent.
 * The spline is completed through a ButtonDoublePressEvent, the escape key or a
 * ButtonPressEvent near the first knot. The latter event closes the spline.
 *
 * Splines segments can be linear or cubic (as in Inkscape). Where there is a nontrivial tangent,
 * the join is smooth.
 * The last knot and tangent can be modified using the keyboard.
 */

class SplineHandler: public InputHandler, public ZoomListener {
public:
    SplineHandler(Control* control, const PageRef& page);
    ~SplineHandler() override;

    std::unique_ptr<xoj::view::OverlayView> createView(xoj::view::Repaintable* parent) const override;

    void onSequenceCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos, double zoom) override;
    void onButtonReleaseEvent(const PositionInputData& pos, double zoom) override;
    void onButtonPressEvent(const PositionInputData& pos, double zoom) override;
    void onButtonDoublePressEvent(const PositionInputData& pos, double zoom) override;
    bool onKeyEvent(GdkEventKey* event) override;

    void finalizeSpline();

    // ZoomListener interface
    void zoomChanged() override;

public:
    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SplineToolView>>& getViewPool() const;

    using Data = SplineHandlerData;
    std::optional<Data> getData() const;

    static auto linearizeSpline(const Data& data) -> std::vector<Point>;

    Range computeTotalRepaintRange(const Data& data, double strokeWidth) const;

    Range computeLastSegmentRepaintRange() const;

private:
    void addKnot(const Point& p);
    void addKnotWithTangent(const Point& p, const Point& t);
    void modifyLastTangent(const Point& t);
    void deleteLastKnotWithTangent();
    void movePoint(double dx, double dy);

    /**
     * @brief Clears out the spline and remove the views. Assumes the spline has at most 1 definitive knot
     */
    void clearTinySpline();

private:
    std::vector<Point> knots{};
    std::vector<Point> tangents{};
    Point currPoint;
    Point buttonDownPoint;  // never snapped to grid
    /**
     * @brief Radius of the knots attractive radius for closing the spline.
     *      Depends on the zoom level during input events
     */
    double knotsAttractionRadius;

    bool isButtonPressed = false;
    bool inFirstKnotAttractionZone = false;
    SnapToGridInputHandler snappingHandler;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::SplineToolView>> viewPool;

    static constexpr double KNOTS_ATTRACTION_RADIUS_IN_PIXELS = 10.0;  // for circling the spline's knots
};
