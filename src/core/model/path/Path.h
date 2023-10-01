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

#include <cstddef>   // for size_t
#include <memory>    // for unique_ptr
#include <optional>  // for optional
#include <utility>   // for pair
#include <vector>    // for vector

#include <cairo.h>  // for cairo_t, cairo_matrix_t

#include "util/Interval.h"
#include "util/Rectangle.h"

class ObjectOutputStream;
class Point;
class ShapeContainer;
struct PaddedBox;

template <class T, size_t N>
class SmallVector;

/**
 * @brief An abstract class to handle paths
 */
class Path {
public:
    Path() = default;
    virtual ~Path() = default;

    enum Type { PIECEWISE_LINEAR, SPLINE };
    virtual Type getType() const = 0;

    /**
     * @brief Iteratable adaptor for segment-based iterations (e.g. for(auto&& segment: spline.segments()) {})
     *
     * This is needed so that two consecutive segments share one knot
     */
    template <class value_type>
    class SegmentIteratable;

    /**
     * @brief Type for parameters of points on a path.
     * Similar to std::pair<size_t, double>, but with named variables
     * The point of parameter (n,t) it the point of parameter t (in [0,1)) in the segment of index n
     */
    struct Parameter {
        Parameter() = default;
        Parameter(size_t index, double t): index(index), t(t) {}
        ~Parameter() = default;
        bool operator==(const Parameter& p) const { return index == p.index && t == p.t; };
        bool operator!=(const Parameter& p) const { return !(*this == p); };
        bool operator<(const Parameter& p) const { return index < p.index || (index == p.index && t < p.t); };
        bool operator>(const Parameter& p) const { return index > p.index || (index == p.index && t > p.t); };
        bool operator<=(const Parameter& p) const { return index < p.index || (index == p.index && t <= p.t); };
        bool operator>=(const Parameter& p) const { return index > p.index || (index == p.index && t >= p.t); };

        bool isValid() const { return t <= 1.0 && t >= 0.0; }

        size_t index;
        double t;
    };

    /**
     * @brief Type for subsections of a stroke
     */
    using SubSection = Interval<Path::Parameter>;

    using IntersectionParametersContainer = SmallVector<Parameter, 4>;

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
     * @brief Set the pressure value of the first knot
     * Assumes the path is not empty.
     */
    void setFirstKnotPressure(double pressure);

    /**
     * @brief Get a vector containing the pressure values of the data points
     * @return The vector
     */
    std::vector<double> getPressureValues() const;

    /**
     * @brief Set the pressure values of the data points using the provided vector
     * @param pressures Vector containing the pressure values to apply
     */
    void setPressureValues(const std::vector<double>& pressures);

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
     * @return The clone
     */
    virtual std::unique_ptr<Path> cloneSection(const SubSection& section) const = 0;

    /**
     * @brief Create a partial clone of a closed path (i.e. data.front() == data.back()) with points
     *     getPoint(startParam) -- ... -- points.back() == points.front() -- ... -- getPoint(endParam)
     * Assumes both startParam and endParam are valid parameters of the path, and endParam < startParam
     */
    virtual std::unique_ptr<Path> cloneCircularSectionOfClosedPath(const Parameter& startParam,
                                                                   const Parameter& endParam) const = 0;

    /**
     * @brief Compute the smallest box containing the path
     * @param fallbackWidth the width used in case the path is pressureless
     * @return The thick bounding box
     */
    virtual Range getThickBoundingBox(double fallbackWidth) const = 0;

    /**
     * @brief Compute the smallest box containing the path. No width taken into account
     * @return The thin bounding box
     */
    virtual Range getThinBoundingBox() const = 0;

    /**
     * @brief Compute the smallest box containing the a subsection of the path.
     * @param fallbackWidth the stroke width to use if the path is pressureless.
     * @return The thin bounding box
     */
    virtual Range getSubSectionBoundingBox(const SubSection& section, const double fallbackWidth) const = 0;

    IntersectionParametersContainer intersectWithPaddedBox(const PaddedBox& box) const;
    virtual IntersectionParametersContainer intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex,
                                                                   size_t lastIndex) const = 0;

    template <typename SegmentType>
    IntersectionParametersContainer intersectWithPaddedBoxTemplate(const PaddedBox& box, size_t firstIndex,
                                                                   size_t lastIndex,
                                                                   SegmentIteratable<const SegmentType> segments) const;

    /**
     * @brief Test if the path is entirely in the given shape
     * @param container Container for the shape
     * @return true if the path is entirely in the shape, false otherwise
     */
    virtual bool isInSelection(ShapeContainer* container) = 0;

    /**
     * @brief Get the squared distance between a point and the path
     * @param p The point
     * @param veryClose Lower bound for the distance
     * @param toFar Upper bound for the distance
     * @return The squared distance, unless this distance is bigger than toFar (in this case, returns toFar) or the
     * distance is smaller than veryClose (returns veryClose)
     *
     * In practice, veryClose and toFar is used to limit the computation time:
     * we stop as soon as we have found a point closer than veryClose and
     * we skip segments that we know to be to far away using some coarse estimate.
     */
    virtual double squaredDistanceToPoint(const Point& p, double veryClose, double toFar) = 0;

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

    virtual void addSectionToCairo(cairo_t* cr, const SubSection& section) const = 0;
    virtual void addCircularSectionToCairo(cairo_t* cr, const Parameter& startParam,
                                           const Parameter& endParam) const = 0;

protected:
    static bool isPointOnBoundary(const Point& p, const xoj::util::Rectangle<double>& r);

    static std::optional<Interval<double>> intersectLineWithRectangle(const Point& p, const Point& q,
                                                                      const xoj::util::Rectangle<double>& rectangle);

protected:
    /**
     * @brief The data: stores points, knots and/or control points, depending on the path's type
     */
    std::vector<Point> data;
};
