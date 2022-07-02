/*
 * Xournal++
 *
 * Handles input of the ruler
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>    // for cairo_t
#include <gdk/gdk.h>  // for GdkEventKey
#include <glib.h>     // for guint32

#include "model/PageRef.h"  // for PageRef
#include "model/Point.h"    // for Point

#include "InputHandler.h"            // for InputHandler
#include "SnapToGridInputHandler.h"  // for SnapToGridInputHandler

class PositionInputData;
class XojPageView;
class XournalView;

enum DIRSET_MODIFIERS { NONE = 0, SET = 1, SHIFT = 1 << 1, CONTROL = 1 << 2 };


class BaseShapeHandler: public InputHandler {
public:
    BaseShapeHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page, bool flipShift = false,
                     bool flipControl = false);

    ~BaseShapeHandler() override;

    void draw(cairo_t* cr) override;

    void onMotionCancelEvent() override;
    bool onMotionNotifyEvent(const PositionInputData& pos) override;
    void onButtonReleaseEvent(const PositionInputData& pos) override;
    void onButtonPressEvent(const PositionInputData& pos) override;
    void onButtonDoublePressEvent(const PositionInputData& pos) override;
    bool onKeyEvent(GdkEventKey* event) override;

private:
    virtual void drawShape(Point& currentPoint, const PositionInputData& pos) = 0;
    DIRSET_MODIFIERS drawModifierFixed = NONE;
    int lastCursor = -1;  // avoid same setCursor
    bool flipShift =
            false;  // use to reverse Shift key modifier action. i.e.  for separate Rectangle and Square Tool buttons.
    bool flipControl = false;  // use to reverse Control key modifier action.

    // to filter out short strokes (usually the user tapping on the page to select it)
    guint32 startStrokeTime{};
    static guint32 lastStrokeTime;  // persist across strokes - allow us to not ignore persistent dotting.

protected:
    /**
     * modifyModifiersByDrawDir
     * @brief 	Toggle shift and control modifiers depending on initial drawing direction.
     */
    void modifyModifiersByDrawDir(double width, double height, bool changeCursor = true);

protected:
    Point currPoint;
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid. See startPoint defined in
                            // derived classes such as CircleHandler.
    bool modShift = false;
    bool modControl = false;
    SnapToGridInputHandler snappingHandler;
};
