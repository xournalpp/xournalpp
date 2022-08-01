/*
 * Xournal++
 *
 * Handles input and optimizes the stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr

#include <cairo.h>    // for cairo_t
#include <gdk/gdk.h>  // for GdkEventKey

#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef

class Control;
class Point;
class Stroke;
class PositionInputData;

/**
 * @brief A base class to handle pointer input
 *
 * The InputHandler receives various events from a XojPageView
 * and updates the XojPageView to display strokes being
 * drawn
 */
class InputHandler: public OverlayBase {
public:
    InputHandler(Control* control, const PageRef& page);
    virtual ~InputHandler();

public:
    /**
     * This method is called from the XojPageView to draw
     * overlays displaying the drawing process.
     * It is called from XojPageView::paintPage(cairo_t* cr, GdkRectangle* rect)
     *
     * @remark The coordinate system is in XojPageView coordinates, scale
     *         it by the current zoom to change to Page coordinates
     */
    virtual void draw(cairo_t* cr) = 0;

    /**
     * This method is called from the XojPageView as soon
     * as the pointer is moved while this InputHandler
     * is active. It is used to update internal data
     * structures and queue repaints of the XojPageView
     * if necessary
     */
    virtual bool onMotionNotifyEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called from the XojPageView when a keypress is detected.
     * It is used to update internal data structures and queue
     * repaints of the XojPageView if necessary.
     */
    virtual bool onKeyEvent(GdkEventKey* event) = 0;

    /**
     * The current input device for stroken, do not react on other devices (linke mices)
     * This method is called from the XojPageView as soon
     * as the pointer is released.
     */
    virtual void onButtonReleaseEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called from the XojPageView as soon
     * as the pointer is pressed.
     */
    virtual void onButtonPressEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called from the XojPageView as soon
     * as the pointer is pressed a second time.
     */
    virtual void onButtonDoublePressEvent(const PositionInputData& pos, double zoom) = 0;

    /**
     * This method is called when an action taken by the pointer is canceled.
     * It is used, for instance, to cancel a stroke drawn when a user starts
     * to zoom on a touchscreen device.
     */
    virtual void onSequenceCancelEvent() = 0;

    Stroke* getStroke() const;

    /**
     * userTapped - experimental feature to take action on filtered draw. See cbDoActionOnStrokeFilter
     */
    bool userTapped = false;

protected:
    [[nodiscard]] static std::unique_ptr<Stroke> createStroke(Control* control);

    static bool validMotion(Point p, Point q);

    /**
     * Smaller movements will be ignored.
     * Expressed in page coordinates
     */
    static constexpr double PIXEL_MOTION_THRESHOLD = 0.3;

protected:
    Control* control;
    PageRef page;

    std::unique_ptr<Stroke> stroke;
};
