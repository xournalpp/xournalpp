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

#include <functional>  // for reference_wrapper
#include <utility>     // for pair
#include <vector>      // for vector

#include <cairo.h>  // for cairo_t

#include "model/Point.h"  // for Point
#include "util/TinyVector.h"

class ShapeContainer;
struct MathVect3;
class Range;

namespace xoj::util {
template <class T>
class Rectangle;
};

/**
 * @brief A class to handle splines segments
 *
 * Every spline segment is defined by two knot points and two control points.
 * This contrasts to the cairo-draw command cairo_curve_to, where everything is relative to the current position of
 * another knot,
 */
class SplineSegment final {
    /**
     * WARNING Do not modify the member variables of this class or add a virtual table!
     * This would break segment-based iteration adapter Path/SegmentIteratable
     */
public:
    SplineSegment() = default;

    using point_t = Point;
    using math_vector_t = MathVect3;

    /**
     * @brief A spline segment from four points
     * @param p the first knot
     * @param fp the first control point
     * @param sp the second control point
     * @param q the second knot
     */
    SplineSegment(const point_t& p, const point_t& fp, const point_t& sp, const point_t& q);

    /**
     * @brief A spline segment from two points and two velocity vectors
     * @param p the first knot
     * @param fTgt the first velocity vector
     * @param sTgt the second velocity vector
     * @param q the second knot
     */
    SplineSegment(const point_t& p, const math_vector_t& fTgt, const math_vector_t& sTgt, const point_t& q);

    /**
     * @brief A constant spline segment from one point
     * @param p the point
     */
    SplineSegment(const point_t& p);

    /**
     * @brief A linear spline segment from two points
     * @param p the first knot
     * @param q the second knot
     */
    SplineSegment(const point_t& p, const point_t& q);

    /**
     * @brief A quadratic spline segment from three points
     * @param p the first knot
     * @param cp the control point
     * @param q the second knot
     */
    SplineSegment(const point_t& p, const point_t& cp, const point_t& q);

    /**
     * @brief draw the spline segment via the cairo_curve_to command
     * @param cr the cairo context on which the spline segment ought to be drawn
     */
    void toCairo(cairo_t* cr) const;

    /**
     * @brief Convert the spline segment to points and fill the given vector. Omit the second knot.
     * @param points Vector of points to fill.
     */
    void toPoints(std::vector<point_t>& points) const;

    /**
     * @brief A small helper structure for points with a parameter
     */
    struct ParametrizedPoint: public point_t {
        ParametrizedPoint() = default;
        ParametrizedPoint(const point_t& p, double t): point_t(p), t(t) {}
        double t;
    };

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
     * @brief Get the y coordinate of the interpolated point at t
     * @param t Parameter value
     * @return The y coordinate of the point of parameter t
     *
     * Use getPoint instead if you also need the other coordinates
     */
    double getY(double t) const;

    /**
     * @brief Get the width coordinate of the interpolated point at t
     * @param t Parameter value
     * @return The width coordinate of the point of parameter t
     *
     * Use getPoint instead if you also need the other coordinates
     */
    [[maybe_unused]] double getZ(double t) const;

    /**
     * @brief Get the interpolated point at t
     * @param t Parameter value
     * @return The point of parameter t
     */
    point_t getPoint(double t) const;

    /**
     * @brief Compute the smallest rectangle containing both knots and both control points
     * @return The rectangle
     *
     * This rectangle contains the entire spline segment. It computation is faster than that of getBoundingBox()
     * On average, the rectangle from getCoarseBoundingBox() is 50% bigger (area wise) than that of getBoundingBox()
     *
     * Benchmark (on modest laptop) 3418428 µs for 1000000 iterations (3.4 µs/it)
     */
    Range getCoarseBoundingBox() const;

    /**
     * @brief Compute the bounding box of the segment (width is not taken into account)
     * @return Bounding box
     *
     * This function is more precise (exact) but slower than getCoarseBoundingBox()
     * Costs 2 calls to rootsOfQuadraticEquation
     *
     * Benchmark (on modest laptop) 6015887 µs for 1000000 iterations (6 µs/it)
     */
    Range getThinBoundingBox() const;

    /**
     * @brief Get a suboptimal upperbound of the width along the spline segment
     * @return An upperbound
     */
    [[maybe_unused]] double getWidthUpperbound() const;

    /**
     * @brief Get the maximal width along the segment
     * @return The maximal width
     */
    [[maybe_unused]] double getMaximalWidth() const;

    /**
     * @brief Compute the exact thick bounding box (with width) of the segment
     * @return Bounding box
     *
     * WARNING: Do not call on pressureless spline segments.
     */
    [[maybe_unused]] Range getThickBoundingBox() const;

    /**
     * @brief Find the parameters (between 0 and 1) corresponding to the points where the spline segment crosses a given
     * horizontal line
     * @param lineY The y coordinate of the vertical line
     * @return The parameters (sorted)
     */
    [[maybe_unused]] TinyVector<double, 3> intersectWithHorizontalLine(double lineY) const;

    /**
     * @brief Find the parameters (between 0 and 1) corresponding to the points where the spline segment crosses a given
     * vertical line
     * @param lineX The x coordinate of the vertical line
     * @return The parameters (sorted)
     */
    [[maybe_unused]] TinyVector<double, 3> intersectWithVerticalLine(double lineX) const;

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
     * This first tests if the rectangle intersects this->getBoundingBox().
     */
    std::vector<double> intersectWithRectangle(const xoj::util::Rectangle<double>& rectangle) const;

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
     * @brief Find the point on the segment that is closest to p.
     * @param p The point
     * @return A pair (t,d) where t is the parameter of the closest point, and d is the square of the smallest distance
     */
    std::pair<double, double> closestPointTo(const point_t& p) const;

    /**
     * @brief Give the square distance between a point and the convex hull of the segment's knots and control points
     * @param p The point
     * @return The squared distance
     */
    double squaredDistanceToHull(const point_t& p) const;

    /**
     * @brief Get the parameter value of the point seg.getPoint(t) in the subsegment seg.subdivide(u).first
     * @param t parameter to transform
     * @param u parameter of subdivision
     *
     * 0 <= t <= u <= 1
     *
     * seg.subdivide(u).first.getPoint(getParameterInFirstSubsegment(u,t)) == seg.getPoint(t)
     */
    [[maybe_unused]] static constexpr inline double getParameterInFirstSubsegment(double t, double u) {
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
    [[maybe_unused]] static constexpr inline double getParameterInSecondSubsegment(double u, double t) {
        return u == 1.0 ? 0.0 : (t - u) / (1 - u);
    }

    /**
     * @brief Tell if the segment is too short to matter
     * @return true if too short
     */
    bool isNegligible();

    /**
     * @brief Get references to points which determine the tangent at the first knot
     * @return Refs to points A and B. The half line starting from A passing by B is the half tangent at the first knot
     */
    std::pair<std::reference_wrapper<const point_t>, std::reference_wrapper<const point_t>> getLeftHalfTangent() const;

    /**
     * @brief Get references to points which determine the tangent at the second knot
     * @return Refs to points A and B. The half line starting from B passing by A is the half tangent
     */
    std::pair<std::reference_wrapper<const point_t>, std::reference_wrapper<const point_t>> getRightHalfTangent() const;

public:
    /**
     * WARNING Keep the member variables in that order and do not add any other (or a virtual table)
     * This would break the segment-based iteration adaptor SegmentIteratable
     */
    /**
     * @brief The first knot of the spline segment.
     */
    point_t firstKnot;

    /**
     * @brief The first control point of the spline segment.
     */
    point_t firstControlPoint;

    /**
     * @brief The second control point of the spline segment.
     */
    point_t secondControlPoint;

    /**
     * @brief The second knot of the spline segment.
     */
    point_t secondKnot;

private:
    static constexpr double FLATNESS_TOLERANCE = 1.0001;
    static constexpr double MIN_KNOT_DISTANCE = 0.3;
    static constexpr double MAX_WIDTH_CHANGE = 0.1;
    static constexpr double MIN_BOX_SIZE = 1e-4;
};
