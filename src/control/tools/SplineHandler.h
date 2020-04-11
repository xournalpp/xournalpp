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

#include "model/Point.h"
#include "model/Snapping.h"
#include "model/SplineSegment.h"
#include "view/DocumentView.h"

#include "InputHandler.h"
#include "SnapToGridInputHandler.h"

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

class SplineHandler: public InputHandler {
public:
    SplineHandler(XournalView* xournal, XojPageView* redrawable, const PageRef& page);
    virtual ~SplineHandler();

    void draw(cairo_t* cr);

    bool onMotionNotifyEvent(const PositionInputData& pos);
    void onButtonReleaseEvent(const PositionInputData& pos);
    void onButtonPressEvent(const PositionInputData& pos);
    void onButtonDoublePressEvent(const PositionInputData& pos);
    virtual bool onKeyEvent(GdkEventKey* event);

private:
    void finalizeSpline();
    void movePoint(double dx, double dy);
    void updateStroke();
    Rectangle<double> computeRepaintRectangle() const;

    // to filter out short strokes (usually the user tapping on the page to select it)
    guint32 startStrokeTime{};
    static guint32 lastStrokeTime;  // persist across strokes - allow us to not ignore persistent dotting.


private:
    std::vector<Point> knots{};
    std::vector<Point> tangents{};
    bool isButtonPressed = false;
    SnapToGridInputHandler snappingHandler;

public:
    void addKnot(const Point& p);
    void addKnotWithTangent(const Point& p, const Point& t);
    void modifyLastTangent(const Point& t);
    void deleteLastKnotWithTangent();
    int getKnotCount() const;

protected:
    DocumentView view;
    Point currPoint;
    Point buttonDownPoint;  // used for tapSelect and filtering - never snapped to grid. See startPoint defined in
                            // derived classes such as CircleHandler.
};
