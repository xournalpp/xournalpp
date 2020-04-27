/*
 * Xournal++
 *
 * A spline segment
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>

#include <gtk/gtk.h>

#include "model/Element.h"
#include "model/Stroke.h"

#include "Point.h"


/**
 * @brief A class to handle splines segments
 *
 * Every spline segment is defined by two knot points and two control points.
 * This contrasts to the cairo-draw command cairo_curve_to, where everything is relative to the current position of
 * another knot,
 */


class SplineSegment {
public:
    SplineSegment() = default;

    /**
     * @brief A linear spline segment from two points
     * @param p the first knot
     * @param q the second knot
     */
    SplineSegment(const Point& p, const Point& q);

    /**
     * @brief A spline segment from four points
     * @param p the first knot
     * @param fp the first control point
     * @param sp the second control point
     * @param q the second knot
     */
    SplineSegment(const Point& p, const Point& fp, const Point& sp, const Point& q);

    /**
     * @brief draw the spline segment via the cairo_curve_to command
     * @param cr the cairo context on which the spline segment ought to be drawn
     */
    void draw(cairo_t* cr) const;

    /**
     * @brief Convert the spline segment to a list of points.
     * @return A point list which represents the spline segment without the end point.
     */
    std::list<Point> toPointSequence() const;

    /**
     * @brief Subdivide the spline into two parts with respect to parameter t.
     * @param t the parameter between 0 and 1, which corresponds to the point where the spline is split
     * @return A pair of two spline segments corresponding to the two parts of the spline
     */
    std::pair<SplineSegment, SplineSegment> subdivide(float t) const;

    /**
     * @brief Interpolate a line segment with respect to parameter t
     * @param t the parameter between 0 and 1, which corresponds to the point on the line segment
     * @param p the starting point of the line segment
     * @param q the end point of the line segment
     * @return A point on the line segment which corresponds to the parameter value t
     */
    static Point linearInterpolate(const Point& p, const Point& q, float t);

    /**
     * @brief checks if the spline segment is flat enough so that it can be drawn as a straight line
     * @return true, if the spline segment is flat enough; false otherwise
     */
    bool isFlatEnough() const;


public:
    /**
     * @brief The first knot of the spline segment.
     */
    Point firstKnot;

    /**
     * @brief The second knot of the spline segment.
     */
    Point secondKnot;

    /**
     * @brief The first control point of the spline segment.
     */
    Point firstControlPoint;

    /**
     * @brief The second control point of the spline segment.
     */
    Point secondControlPoint;
};
