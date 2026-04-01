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

#include <list>     // for list
#include <utility>  // for pair

#include <cairo.h>  // for cairo_t

#include "model/Point.h"  // for Point


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
     * @param usePressure If true, interpolate the pressure along the spline. Default: false
     * @return A point list which represents the spline segment without the end point.
     */
    std::list<Point> toPointSequence(bool usePressure = false) const;

    /**
     * @brief Subdivide the spline into two parts with respect to parameter t.
     * @param t the parameter between 0 and 1, which corresponds to the point where the spline is split
     * @param usePressure If true, infer pressure values for the resulting SplineSegments' endpoints. Default: false.
     * @return A pair of two spline segments corresponding to the two parts of the spline
     */
    std::pair<SplineSegment, SplineSegment> subdivide(float t, bool usePressure = false) const;

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
     * @param usePressure If true, return false if the endpoints' pressure values are to far appart. Default: false.
     * @return true, if the spline segment is flat enough; false otherwise
     */
    bool isFlatEnough(bool usePressure = false) const;


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
