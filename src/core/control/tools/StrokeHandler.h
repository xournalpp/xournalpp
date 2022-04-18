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

#include "view/View.h"

#include "InputHandler.h"
#include "SnapToGridInputHandler.h"

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
    StrokeHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    ~StrokeHandler() override = default;

    void draw(cairo_t* cr) override;

    void onMotionCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos) override;
    void onButtonReleaseEvent(const PositionInputData& pos) override;
    void onButtonPressEvent(const PositionInputData& pos) override;
    void onButtonDoublePressEvent(const PositionInputData& pos) override;
    bool onKeyEvent(GdkEventKey* event) override;

    /**
     * @brief Add a straight line to the stroke (if the movement is valid).
     * The line may be subdivided into smaller segments if the pressure variation is too big.
     * @param point The endpoint of the added line
     */
    void paintTo(const Point& point);

    /**
     * @brief paints a single dot
     */
    void paintDot(cairo_t* cr, const double x, const double y, const double width) const;

protected:
    /**
     * @brief Unconditionally add a segment to the stroke.
     * Warning: it does not set the width properly nor test if the motion is valid. Use paintTo instead.
     * @param point The endpoint of the added segment
     */
    void drawSegmentTo(const Point& point);

    void strokeRecognizerDetected(Stroke* recognized, Layer* layer);

protected:
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid.
    SnapToGridInputHandler snappingHandler;

private:
    /**
     * @brief Create and initialize the mask
     * The mask is used for strokes that do not require a full redraw at each input event.
     * For those strokes, whenever a new input event is received, the new segment is simply added to the mask.
     * The mask is then blitted upon a call to `draw`.
     *
     * A stroke requires a full redraw if
     *      * it has a filling (the filling can not be computed simply from just the last segment)
     *      * or it has dashes (to get the dash offset right)
     *
     * Nb: the dashed exception could be avoided if we recorded the dash offset (= the stroke's length so far)
     */
    void createMask();

    // Helper class to safely handle cairo surface and context
    class Mask {
    public:
        Mask() = delete;
        Mask(const Mask&) = delete;
        Mask(Mask&&) = delete;
        Mask& operator=(const Mask&) = delete;
        Mask& operator=(Mask&&) = delete;

        Mask(int width, int height);
        ~Mask() noexcept;

        /**
         * The masking surface
         */
        cairo_surface_t* surf = nullptr;

        /**
         * And the corresponding cairo_t*
         */
        cairo_t* cr = nullptr;
    };
    std::optional<Mask> mask;

    // to filter out short strokes (usually the user tapping on the page to select it)
    guint32 startStrokeTime{};
    static guint32 lastStrokeTime;  // persist across strokes - allow us to not ignore persistent dotting.

    /**
     * @brief Pointer to the Stabilizer instance
     */
    std::unique_ptr<StrokeStabilizer::Base> stabilizer;

    bool hasPressure;
    bool firstPointPressureChange = false;

    friend class StrokeStabilizer::Active;

    static constexpr double MAX_WIDTH_VARIATION = 0.3;
};
