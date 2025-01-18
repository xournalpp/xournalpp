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

class Control;
class Layer;
class PositionInputData;
class Stroke;

namespace xoj::util {
template <class T>
class DispatchPool;
};

namespace xoj::view {
class OverlayView;
class Repaintable;
class StrokeToolView;
};  // namespace xoj::view

namespace StrokeStabilizer {
class Base;
class Active;
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
    bool onKeyPressEvent(const KeyEvent& event) override;
    bool onKeyReleaseEvent(const KeyEvent& event) override;

    /**
     * @brief Add a straight line to the stroke (if the movement is valid).
     * The line may be subdivided into smaller segments if the pressure variation is too big.
     * @param point The endpoint of the added line
     */
    void paintTo(Point point);

    auto createView(xoj::view::Repaintable* parent) const -> std::unique_ptr<xoj::view::OverlayView> override;

    const std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolView>>& getViewPool() const;

protected:
    /**
     * @brief Unconditionally add a segment to the stroke.
     * Warning: it does not set the width properly nor test if the motion is valid. Use paintTo instead.
     * @param point The endpoint of the added segment
     */
    void drawSegmentTo(const Point& point);

    void strokeRecognizerDetected(std::unique_ptr<Stroke> recognized, Layer* layer);

protected:
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid.
    SnapToGridInputHandler snappingHandler;

private:
    /**
     * @brief Pointer to the Stabilizer instance
     */
    std::unique_ptr<StrokeStabilizer::Base> stabilizer;

    std::shared_ptr<xoj::util::DispatchPool<xoj::view::StrokeToolView>> viewPool;

    bool hasPressure;

    friend class StrokeStabilizer::Active;

    static constexpr double MAX_WIDTH_VARIATION = 0.3;
};
