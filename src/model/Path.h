/**
 * Xournal++
 *
 * A path
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>

#include <cairo.h>

#include "Rectangle.h"

class ObjectOutputStream;
class Point;
class ShapeContainer;

#define EXTRA_CAREFUL
/**
 * @brief An abstract class to handle paths
 */
class Path {
public:
    Path() = default;
    virtual ~Path() = default;

    Path& operator=(const std::vector<Point>& vector);

    enum Type { PIECEWISE_LINEAR, SPLINE };
    virtual Type getType() const = 0;

    /**
     * @brief Serialize the path
     * @param out The output stream to serialize to
     */
    virtual void serialize(ObjectOutputStream& out) const = 0;

    /**
     * @brief Get the data of the path
     * @return Reference to the data
     */
    const std::vector<Point>& getData() const;

    /**
     * @brief Get the last knot
     * @return The last knot
     */
    const Point& getLastKnot() const;

    /**
     * @brief Get the first knot
     * @return The first knot
     */
    const Point& getFirstKnot() const;

    /**
     * @brief Type for parameters of points on a path.
     * Similar to std::pair<size_t, double>, but with named variables
     */
    struct Parameter {
        Parameter(size_t index, double t): index(index), t(t) {}
        ~Parameter() = default;
        bool operator==(const Parameter& p) const { return index == p.index && t == p.t; };
        bool operator!=(const Parameter& p) const { return !(*this == p); };
        bool operator<(const Parameter& p) const { return index < p.index || (index == p.index && t < p.t); };
        bool operator>(const Parameter& p) const { return index > p.index || (index == p.index && t > p.t); };
        bool operator<=(const Parameter& p) const { return index < p.index || (index == p.index && t <= p.t); };
        bool operator>=(const Parameter& p) const { return index > p.index || (index == p.index && t >= p.t); };

        size_t index;
        double t;
    };

    /**
     * @brief Get the point with given parameter on the path
     * @param parameter The point's parameter
     * @return The point
     */
    virtual Point getPoint(const Parameter& parameter) const = 0;

    /**
     * @brief Clone the path
     * @return The clone
     */
    virtual std::unique_ptr<Path> clone() const = 0;

    /**
     * @brief Clone the section of the path between the given parameters
     * @param lowerBound Beginning of the cloned spline
     * @param upperBound End of the cloned spline
     * @return The clone
     */
    virtual std::unique_ptr<Path> cloneSection(const Parameter& lowerBound, const Parameter& upperBound) const = 0;

    /**
     * @brief Compute the smallest box containing the path. No width taken into account
     * @return The thin bounding box
     */
    virtual Rectangle<double> getThinBoundingBox() const = 0;

    /**
     * @brief Find the parameters corresponding to the points where the path crosses in
     * or out of the given rectangle
     * @param rectangle The rectangle
     * @return The parameters (sorted)
     *
     * Warning: this function does not test if the rectangle intersects this->getBoundingBox().
     * For optimization purposes, this test should be performed beforehand by the caller.
     */
    std::vector<Parameter> intersectWithRectangle(const Rectangle<double>& rectangle) const;

    /**
     * @brief Find the parameters within a certain interval corresponding to the points where the path crosses in
     * or out of the given rectangle
     * @param rectangle The rectangle
     * @param begin The lower bound of the interval
     * @param end The upper bound of the interval
     * @return The parameters (sorted)
     *
     * Warning: this function does not test if the rectangle intersects this->getBoundingBox().
     * For optimization purposes, this test should be performed beforehand by the caller.
     */
    virtual std::vector<Parameter> intersectWithRectangle(const Rectangle<double>& rectangle, size_t firstIndex,
                                                          size_t lastIndex) const = 0;

    /**
     * @brief Test if the path is entirely in the given shape
     * @param container Container for the shape
     * @return true if the path is entirely in the shape, false otherwise
     */
    virtual bool isInSelection(ShapeContainer* container) = 0;

    /**
     * @brief Test if the data vector is empty
     * @return true if data is empty, false otherwize
     */
    bool empty() const;

    /**
     * @brief Get the number of segments. The type of segments depends on the type of the path
     * @return The number of segments
     *
     * Nb: When nbSegments() == 0, the path's first knot may or may not be set. Use empty() instead
     */
    virtual size_t nbSegments() const = 0;

    /**
     * @brief Clear all the data
     */
    void clear();

    /**
     * @brief If n < size(), resize the path to n segments
     * @param n Number of segments to keep
     *
     * Nb: resize(0) will not erase the first knot. Use clear() instead
     */
    virtual void resize(size_t n) = 0;

    void freeUnusedPointItems();

    /**
     * @brief Move (translate) the path
     * @param dx Shift in the x coordinate
     * @param dy Shift in the y coordinate
     */
    void move(double dx, double dy);

    /**
     * @brief Rotate the path around the point of coordinates (x0, y0) by the angle th
     * @param x0 X coordinate of the rotation center
     * @param y0 Y coordinate of the rotation center
     * @param th Angle of the rotation
     * @return The matrix applied to every point
     */
    cairo_matrix_t rotate(double x0, double y0, double th);

    /**
     * @brief Rescale the path around a reference point (x0, y0) and along two arbitrary orthogonal axis
     * @param x0 X coordinate of reference point
     * @param y0 Y coordinate of reference point
     * @param fx Scale factor along the first rescaling axis
     * @param fy Scale factor along the second rescaling axis
     * @param rotation Angle between the horizontal and the first rescaling axis
     * @param restoreLineWidth If true, the width is unaffected by the rescaling. Otherwise, the width is rescaled.
     * @return The matrix applied to every point and the rescaling ratio applied to the width.
     */
    std::pair<cairo_matrix_t, double> scale(double x0, double y0, double fx, double fy, double rotation,
                                            bool restoreLineWidth);

    /**
     * @brief Add the path to cairo. Does not actually paint.
     * @param cr The cairo instance
     */
    virtual void addToCairo(cairo_t* cr) const = 0;

protected:
    static bool isPointOnBoundary(const Point& p, const Rectangle<double>& r);

    /**
     * @brief The data: stores points, knots and/or control points, depending on the path's type
     */
    std::vector<Point> data;
};
