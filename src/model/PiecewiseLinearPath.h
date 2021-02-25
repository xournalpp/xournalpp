/**
 * Xournal++
 *
 * A spline
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
/**
 * The algorithms implemented in the class PiecewiseLinearPath::SchneiderApproximater come from
 *
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */

#pragma once

#include "gtk/gtk.h"

#include "MathVect.h"
#include "Path.h"
#include "Point.h"

class ObjectInputStream;

struct LineSegment {
    Point startPoint;
    Point endPoint;
};

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
    PiecewiseLinearPath& operator=(const std::vector<Point>& vector);

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

    /**
     * @brief Set the starting point of the path
     * @param p The new starting point
     */
    void setFirstPoint(const Point& p);

    /**
     * @brief Add a line segment
     * @param q Endpoint of the added line segment
     */
    void addLineSegmentTo(const Point& q);

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
    std::unique_ptr<Path> cloneSection(const Parameter& lowerBound, const Parameter& upperBound) const override;

    /**
     * @brief Compute the bounding box without taking pressure/width into account
     * @return The bounding box
     */
    Rectangle<double> getThinBoundingBox() const override;

    /**
     * @brief Compute the bounding box taking pressure/width into account
     * @return The thick bounding box
     */
    Rectangle<double> getThickBoundingBox() const;

    /**
     * @brief Compute two bounding boxes: the first one without taking pressure/width into account, the second with
     * pressure/width
     * @return A pair (thin bounding box, thick bounding box)
     */
    std::pair<Rectangle<double>, Rectangle<double>> getBoundingBoxes() const;

    /**
     * @brief Rescale the pressure values of all the points
     * @param factor The rescaling factor
     */
    void scalePressure(double factor);

    [[deprecated]] void setPressure(const vector<double>& pressure);
    [[deprecated]] void setSecondToLastPressure(double pressure);

    void clearPressure();

    /**
     */
    void extrapolateLastPressureValue();

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

    /**
     * @brief Find the parameters corresponding to the points where the PL path crosses in or out of the given rectangle
     * @param rectangle The rectangle
     * @return The parameters (sorted)
     *
     * Warning: this function does not test if the rectangle intersects this->getBoundingBox().
     * For optimization purposes, this test should be performed beforehand by the caller.
     */
    std::vector<Parameter> intersectWithRectangle(const Rectangle<double>& rectangle) const;

    /**
     * @brief Find the parameters within a certain interval corresponding to the points where the PL path crosses in
     * or out of the given rectangle
     * @param rectangle The rectangle
     * @param begin The lower bound of the interval
     * @param end The upper bound of the interval
     * @return The parameters (sorted)
     *
     * Warning: this function does not test if the rectangle intersects this->getBoundingBox().
     * For optimization purposes, this test should be performed beforehand by the caller.
     */
    std::vector<Parameter> intersectWithRectangle(const Rectangle<double>& rectangle, size_t firstIndex,
                                                  size_t lastIndex) const override;

    /**
     * @brief Add the path to cairo. Does not actually paint.
     * @param cr The cairo instance
     */
    void addToCairo(cairo_t* cr) const override;

private:
    static std::vector<double> intersectSegmentWithRectangle(const Point& p, const Point& q,
                                                             const Rectangle<double>& rectangle);
};
