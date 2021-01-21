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

#include "Element.h"
#include "MathVect.h"
#include "Point.h"

/**
 * @brief A class containing a partial (or relative) spline segment: the initial knot is missing
 */ 
class PartialSplineSegment {
public:
    PartialSplineSegment() = default;
    /**
     * @brief Create a linear spline segment from p to q
     */
    PartialSplineSegment(const Point& p, const Point& q);
    
    PartialSplineSegment(const Point& fp, const Point& sp, const Point& q);
    
    PartialSplineSegment(const Point& p, const MathVect3& fVelocity, const MathVect3 sVelocity, const Point& q);

    /**
     * @brief Subdivide a partial spline segment into two
     * @param firstKnot Used as a first knot
     * @param t Parameter (between 0 and 1) at which we cut the spline
     * @return Pair of the resulting two partial spline segments
     */
    std::pair<PartialSplineSegment, PartialSplineSegment> subdivide(const Point& firstKnot, double t) const;
    
    /**
     * @brief Convert the spline segment to points and push them to the given vector. Do not push the first knot.
     * @param firstKnot The first knot used for interpolation
     * @param points Vector of points to which the interpolated points will be pushed
     */
    void toPoints(const Point& firstKnot, std::vector<Point>& points) const;
    
    /**
     * @brief Get the x coordinate of the interpolated point at t
     * @param firstKnot Used as the first knot of the segment
     * @param t Parameter value
     * @return The x coordinate of the point of parameter t
     * 
     * Use getPoint instead if you also need the other coordinates
     */
    double getX(const Point& firstKnot, double t) const;    

    /**
     * @brief Get the x coordinate of the interpolated point at t
     * @param firstKnot Used as the first knot of the segment
     * @param t Parameter value
     * @return The x coordinate of the point of parameter t
     * 
     * Use getPoint instead if you also need the other coordinates
     */
    double getY(const Point& firstKnot, double t) const;
    
    /**
     * @brief Get the interpolated point at t
     * @param firstKnot Used as the first knot of the segment
     * @param t Parameter value
     * @return The point of parameter t
     */
    Point getPoint(const Point& firstKnot, double t) const;
    
    /**
     * @brief Compute the bounding box of the segment
     * @param firstKnot Used as the first knot of the segment
     * @return Bounding box
     */
    Rectangle<double> getBoundingBox(const Point& firstKnot) const;

private:
    /**
     * @brief Compute the roots of the polynomial equation a * t^2 + 2 * b * t + c
     * @param a Quadratic coefficient
     * @param b Half of linear coefficient
     * @param c Constant coefficient
     * @return Vector containing the roots
     */
    static std::vector<double> rootsOfPolynomialEquation(double a, double b, double c);
    
protected:
    static constexpr double FLATNESS_TOLERANCE = 1.0001;
    static constexpr double MIN_KNOT_DISTANCE = 0.3;
    static constexpr double MAX_WIDTH_CHANGE = 0.1;
    
public:
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

/**
 * @brief A class to handle splines segments
 *
 * Every spline segment is defined by two knot points and two control points.
 * This contrasts to the cairo-draw command cairo_curve_to, where everything is relative to the current position of
 * another knot,
 */
class SplineSegment: public PartialSplineSegment {
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
     * @brief A spline segment from one point and a PartialSplineSegment
     * @param p the first knot
     * @param partial the PartialSplineSegment
     */
    SplineSegment(Point& p, const PartialSplineSegment& partial);

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
     * @brief Subdivide the spline into two parts with respect to parameter t.
     * @param t the parameter between 0 and 1, which corresponds to the point where the spline is split
     * @return A pair of two spline segments corresponding to the two parts of the spline
     */
    std::pair<SplineSegment, SplineSegment> subdivide(float t) const;

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
    
//     /**
//      * @brief The length of the spline segment (once it has been computed)
//      */
//     double length = -1.0;
};
