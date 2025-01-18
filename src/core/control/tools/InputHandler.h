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

#include <gdk/gdk.h>  // for GdkEventKey

#include "model/OverlayBase.h"
#include "model/PageRef.h"  // for PageRef

class Control;
class Point;
class Stroke;
class PositionInputData;
struct KeyEvent;

namespace xoj::view {
class OverlayView;
class Repaintable;
};  // namespace xoj::view

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
    virtual bool onKeyPressEvent(const KeyEvent& event) = 0;
    virtual bool onKeyReleaseEvent(const KeyEvent& event) = 0;

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

    virtual std::unique_ptr<xoj::view::OverlayView> createView(xoj::view::Repaintable* parent) const = 0;

    Stroke* getStroke() const;

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
