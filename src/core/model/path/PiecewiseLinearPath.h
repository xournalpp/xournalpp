/**
 * Xournal++
 *
 * A piecewise linear path
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <functional>  // for reference_wrapper
#include <utility>     // for pair, forward

#include "model/Point.h"

#include "Path.h"

class ObjectInputStream;

template <class T, size_t N>
class TinyVector;

/**
 * @brief A class to handle piecewise linear (PL) paths (i.e. a succession of line segments)
 */
class PiecewiseLinearPath: public Path {
public:
    PiecewiseLinearPath() = default;
    PiecewiseLinearPath(const PiecewiseLinearPath&) = default;
    PiecewiseLinearPath(PiecewiseLinearPath&&) = default;
    virtual ~PiecewiseLinearPath() override = default;

    PiecewiseLinearPath& operator=(const PiecewiseLinearPath& path) = default;
    PiecewiseLinearPath& operator=(PiecewiseLinearPath&& path) = default;

    PiecewiseLinearPath(const std::vector<Point>& vector);
    PiecewiseLinearPath(std::vector<Point>&& vector);
    PiecewiseLinearPath& operator=(const std::vector<Point>& vector);
    PiecewiseLinearPath& operator=(std::vector<Point>&& vector);

    Type getType() const override { return PIECEWISE_LINEAR; }

    /**
     * @brief Create a PL path with first point firstPoint
     * @param firstPoint The first point
     */
    PiecewiseLinearPath(const Point& firstPoint);

    /**
     * @brief Create a PL path with first point prescribed and reserves the space for size segments
     * @param firstPoint the first point of the PL path
     * @param size Number of pre-allocated segments
     */
    PiecewiseLinearPath(const Point& firstPoint, size_t size);

    /**
     * @brief Create a PL path with a single segment
     * @param firstPoint The first point
     * @param secondPoint The second point
     */
    PiecewiseLinearPath(const Point& firstPoint, const Point& secondPoint);

    /**
     * @brief Read the PL path from a stream
     * @param in The input stream
     */
    PiecewiseLinearPath(ObjectInputStream& in);

    /**
     * @brief Serialize the PL path
     * @param out The output stream to serialize to
     */
    void serialize(ObjectOutputStream& out) const override;

    struct LineSegment final {
        using point_t = Point;
        /**
         * WARNING Do not modify the member variables of this class or add a virtual table!
         * This would break segment-based iteration adapter Path/SegmentIteratable
         */
        point_t firstKnot;
        point_t secondKnot;
        TinyVector<double, 2> intersectWithRectangle(const xoj::util::Rectangle<double> rectangle) const;

        std::pair<std::reference_wrapper<const point_t>, std::reference_wrapper<const point_t>> getLeftHalfTangent()
                const;
        std::pair<std::reference_wrapper<const point_t>, std::reference_wrapper<const point_t>> getRightHalfTangent()
                const;
        point_t getPoint(double t) const;
    };

    /**
     * @brief Get an iteratable adaptor for segment-based iterations.
     * @return The adaptor
     * Warning, the returned adaptor will be invalidated if something is added or removed from this->data
     */
    SegmentIteratable<LineSegment> segments();

    /**
     * @brief Get an iteratable adaptor for segment-based iterations.
     * @return The adaptor
     * Warning, the returned adaptor will be invalidated if something is added or removed from this->data
     */
    SegmentIteratable<const LineSegment> segments() const;

    /**
     * @brief Set the starting point of the path
     * @param p The new starting point
     */
    void setFirstPoint(const Point& p);

    /**
     * @brief Add a line segment to the inplace constructed Point
     */
    template <class... Args>
    void addLineSegmentTo(Args&&... args) {
        data.emplace_back(std::forward<Args>(args)...);
    }

    /**
     * @brief Append then path `other` to `this`
     * Assumes other.getFirstKnot() == this->getLastKnot()
     */
    void append(const PiecewiseLinearPath& other);

    /**
     * @brief Closes the path by adding a line segment to its first knot.
     */
    void close();

    /**
     * @brief Get a segment
     * @param index The index of the segment
     * @return The segment
     */
    const LineSegment& getSegment(size_t index) const;

    /**
     * @brief Get the point with given parameter on the PL path
     * @param parameter The point's parameter
     * @return The point
     */
    Point getPoint(const Parameter& parameter) const override;

    /**
     * @brief Get the point with given index in the data vector
     * @param index The point's parameter
     * @return The point
     */
    const Point& getPoint(const size_t index) const;

    /**
     * @brief Clone the path
     * @return The clone
     */
    std::unique_ptr<Path> clone() const override;

    /**
     * @brief Clone the section of the PL path between the given parameters
     * @param lowerBound Beginning of the cloned PL path
     * @param upperBound End of the cloned PL path
     * @return The clone
     */
    std::unique_ptr<Path> cloneSection(const SubSection& section) const override;

    /**
     * @brief Create a partial clone of a closed path (i.e. data.front() == data.back()) with points
     *     getPoint(startParam) -- ... -- points.back() == points.front() -- ... -- getPoint(endParam)
     * Assumes both startParam and endParam are valid parameters of the path, and endParam < startParam
     */
    std::unique_ptr<Path> cloneCircularSectionOfClosedPath(const Parameter& lowerBound,
                                                           const Parameter& upperBound) const override;

    /**
     * @brief Compute the smallest box containing the path
     * @param fallbackWidth the width used in case the path is pressureless
     * @return The thick bounding box
     */
    Range getThickBoundingBox(double fallbackWidth) const override;

    /**
     * @brief Compute the bounding box without taking pressure/width into account
     * @return The thin bounding box
     */
    Range getThinBoundingBox() const override;

    /**
     * @brief Compute the smallest box containing the a subsection of the path. No width taken into account
     * @return The thin bounding box
     */
    Range getSubSectionBoundingBox(const SubSection& section, const double fallbackWidth) const override;

    void setSecondToLastPressure(double pressure);
    void setLastPressure(double pressure);

    /**
     */
    void extrapolateLastPressureValue();

    /**
     * @brief Compute the average pressure value of the points
     * @return The average pressure
     */
    double getAveragePressure();

    /**
     * @brief Get the number of line segments
     * @return The number of line segments
     *
     * Nb: When nbSegments() == 0, the path's first point may or may not be set
     */
    size_t nbSegments() const override;

    /**
     * @brief If n < size(), resize the PL path to n segments
     * @param n Number of segments to keep
     *
     * Nb: resize(0) will not erase the first point.
     */
    void resize(size_t n) override;

    //     void intersectWithPaddedBoxMainLoop(IntersecterContext& context, size_t firstIndex, size_t lastIndex, const
    //     xoj::util::Rectangle<double>& innerBox, const xoj::util::Rectangle<double>& outerBox) const override;

    IntersectionParametersContainer intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex,
                                                           size_t lastIndex) const override;


    //     /**
    //      * @brief Find the parameters corresponding to the points where the PL path crosses in or out of the given
    //      rectangle
    //      * @param rectangle The rectangle
    //      * @return The parameters (sorted)
    //      *
    //      * Warning: this function does not test if the rectangle intersects this->getBoundingBox().
    //      * For optimization purposes, this test should be performed beforehand by the caller.
    //      */
    //     std::vector<Parameter> intersectWithRectangle(const xoj::util::Rectangle<double>& rectangle) const;
    //
    //     /**
    //      * @brief Find the parameters within a certain interval corresponding to the points where the PL path crosses
    //      in
    //      * or out of the given rectangle
    //      * @param rectangle The rectangle
    //      * @param begin The lower bound of the interval
    //      * @param end The upper bound of the interval
    //      * @return The parameters (sorted)
    //      *
    //      * Warning: this function does not test if the rectangle intersects this->getBoundingBox().
    //      * For optimization purposes, this test should be performed beforehand by the caller.
    //      */
    //     std::vector<Parameter> intersectWithRectangle(const xoj::util::Rectangle<double>& rectangle, size_t
    //     firstIndex,
    //                                                   size_t lastIndex) const override;

    /**
     * @brief Test if the path is entirely in the given shape
     * @param container Container for the shape
     * @return true if the path is entirely in the shape, false otherwise
     */
    bool isInSelection(ShapeContainer* container) override;

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
     * we skip segments that we know to be too far away using some coarse estimate.
     */
    virtual double squaredDistanceToPoint(const Point& p, double veryClose, double toFar) override;

    /**
     * @brief Add the path to cairo. Does not actually paint.
     * @param cr The cairo instance
     */
    void addToCairo(cairo_t* cr) const override;

    void addSectionToCairo(cairo_t* cr, const SubSection& section) const override;
    void addCircularSectionToCairo(cairo_t* cr, const Parameter& startParam, const Parameter& endParam) const override;
};
