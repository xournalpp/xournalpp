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

// #include <list>

#include <gtk/gtk.h>

#include "MathVect.h"
#include "Point.h"

class ShapeContainer;

/**
 * @brief A small helper structure for points with a parameter
 */
struct ParametrizedPoint: public Point {
    ParametrizedPoint() = default;
    ParametrizedPoint(const Point& p, double t): Point(p), t(t) {}
    double t;
};

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
     * @brief Convert the spline segment to points and fill the given vector. Omit the second knot.
     * @param points Vector of points to fill.
     */
    void toPoints(std::vector<Point>& points) const;

    /**
     * @brief Convert the spline segment to parametrized points and fill the given vector. Omit the second knot.
     * @param points Vector of parametrized points to fill.
     * @param start Parameter value assigned to the first knot
     * @param end Parameter value assigned to the second knot
     */
    void toParametrizedPoints(std::vector<ParametrizedPoint>& points, double start = 0.0, double end = 1.0) const;

    /**
     * @brief Subdivide the spline into two parts with respect to parameter t.
     * @param t the parameter between 0 and 1, which corresponds to the point where the spline is split
     * @return A pair of two spline segments corresponding to the two parts of the spline
     */
    std::pair<SplineSegment, SplineSegment> subdivide(double t) const;

    /**
     * @brief Compute the subsegment corresponding to the parameter interval [tMin, tMax]
     * @param tMin Interval lower bound
     * @param tMax Interval upper bound
     * @return The segment restricted to [tMin, tMax]
     */
    SplineSegment getSubsegment(double tMin, double tMax) const;

    /**
     * @brief checks if the spline segment is flat enough so that it can be drawn as a straight line
     * @return true, if the spline segment is flat enough; false otherwise
     */
    bool isFlatEnough() const;

    /**
     * @brief Get the x coordinate of the interpolated point at t
     * @param t Parameter value
     * @return The x coordinate of the point of parameter t
     *
     * Use getPoint instead if you also need the other coordinates
     */
    double getX(double t) const;

    /**
     * @brief Get the x coordinate of the interpolated point at t
     * @param t Parameter value
     * @return The x coordinate of the point of parameter t
     *
     * Use getPoint instead if you also need the other coordinates
     */
    double getY(double t) const;

    /**
     * @brief Get the interpolated point at t
     * @param t Parameter value
     * @return The point of parameter t
     */
    Point getPoint(double t) const;

    /**
     * @brief Compute the smallest rectangle containing both knots and both control points
     * @return The rectangle
     *
     * This rectangle contains the entire spline segment. It computation is faster than that of getBoundingBox()
     * On average, the rectangle from getCoarseBoundingBox() is 25% bigger (area wise) than that of getBoundingBox()
     */
    Rectangle<double> getCoarseBoundingBox() const;

    /**
     * @brief Compute the bounding box of the segment
     * @return Bounding box
     *
     * This function is more precise (exact) but slower than getCoarseBoundingBox()
     * Cost 2 calls to rootsOfQuadraticEquation
     */
    Rectangle<double> getBoundingBox() const;

    /**
     * @brief Find the parameters (between 0 and 1) corresponding to the points where the spline segment crosses a given
     * horizontal line
     * @param lineY The y coordinate of the vertical line
     * @return The parameters (sorted)
     */
    [[maybe_unused]] std::vector<double> intersectWithHorizontalLine(double lineY) const;

    /**
     * @brief Find the parameters (between 0 and 1) corresponding to the points where the spline segment crosses a given
     * vertical line
     * @param lineX The x coordinate of the vertical line
     * @return The parameters (sorted)
     */
    [[maybe_unused]] std::vector<double> intersectWithVerticalLine(double lineX) const;

    /**
     * @brief Find the parameters (0 < t <= 1) corresponding to the points where the spline segment crosses
     * in or out of the given rectangle
     * @param rectangle The rectangle
     * @param min The lower bound of the interval
     * @param max The upper bound of the interval
     * @return The (sorted) parameters t (with min < t <= max)
     *
     * Nb: The values of min and max default so that the obtained parameters satisfy 0 < t <= 1.
     *
     * Warning: this does not test if the rectangle intersects this->getBoundingBox() or this->getCoarseBoundingBox()
     * For optimization purposes, this test should be performed beforehand by the calling function.
     */
    std::vector<double> intersectWithRectangle(const Rectangle<double>& rectangle) const;

    /**
     * @brief Test if the spline segment is entirely in the given shape, assuming the first knot is in the shape
     * @param container Container for the shape
     * @param assumeSecondKnotIn If true, assume the second knot is inside the shape
     * @return true if the segment is entirely in the shape, false otherwise
     *
     * Warning: this function always assumes the first knot is already known to be inside the shape
     */
    bool isTailInSelection(ShapeContainer* container, bool assumeSecondKnotIn) const;

    /**
     * @brief Compute the smallest distance between the given point and the segment
     */
    double distanceToPoint(const Point& p) const;

    /**
     * @brief Get the parameter value of the point seg.getPoint(t) in the subsegment seg.subdivide(u).first
     * @param t parameter to transform
     * @param u parameter of subdivision
     *
     * 0 <= t <= u <= 1
     *
     * seg.subdivide(u).first.getPoint(getParameterInFirstSubsegment(u,t)) == seg.getPoint(t)
     */
    [[maybe_unused]] static inline double getParameterInFirstSubsegment(double t, double u) {
        return u == 0.0 ? 0.0 : t / u;
    }

    /**
     * @brief Get the parameter value of the point seg.getPoint(t) in the subsegment seg.subdivide(u).second
     * @param u parameter of subdivision
     * @param t parameter to transform
     *
     * 0 <= u <= t <= 1
     *
     * seg.subdivide(u).second.getPoint(getParameterInSecondSubsegment(u,t)) == seg.getPoint(t)
     */
    [[maybe_unused]] static inline double getParameterInSecondSubsegment(double u, double t) {
        return u == 1.0 ? 0.0 : (t - u) / (1 - u);
    }

public:
    /**
     * @brief The first knot of the spline segment.
     */
    Point firstKnot;

    /**
     * @brief The first control point of the spline segment.
     */
    Point firstControlPoint;

    /**
     * @brief The second control point of the spline segment.
     */
    Point secondControlPoint;

    /**
     * @brief The second knot of the spline segment.
     */
    Point secondKnot;

private:
    static constexpr double FLATNESS_TOLERANCE = 1.0001;
    static constexpr double MIN_KNOT_DISTANCE = 0.3;
    static constexpr double MAX_WIDTH_CHANGE = 0.1;
};
