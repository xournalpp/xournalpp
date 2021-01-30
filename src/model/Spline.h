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
 * The algorithms implemented in the class Spline::SchneiderApproximater come from
 *
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */

#pragma once

#include <list>

#include <gtk/gtk.h>

#include "Element.h"
#include "MathVect.h"
#include "Point.h"
#include "SplineSegment.h"

/**
 * @brief A class to handle splines
 * A spline is only assumed to be continuous.
 */
class Spline {
public:
    Spline() = default;
    Spline& operator=(const Spline& spline) = default;

    /**
     * @brief Create a spline with first knot firstKnot
     * @param firstKnot The first knot
     */
    Spline(const Point& firstKnot);

    /**
     * @brief Create a spline with first knot and first segment
     * @param firstKnot the first knot of the spline
     * @param firstSegment the first segment of the spline
     */
    Spline(const Point& firstKnot, const PartialSplineSegment& firstSegment);

    /**
     * @brief Create a spline with first knot prescribed and reserves the space for size segments
     * @param firstKnot the first knot of the spline
     * @param size Number of pre-allocated segments
     */
    Spline(const Point& firstKnot, size_t size);

    /**
     * @brief Add a line segment
     * @param q Endpoint of the added line segment
     */
    void addLineSegment(const Point& q);

    /**
     * @brief Add a quadratic spline segment
     * @param cp The control point of the quadratic segment
     * @param q The endpoint of the quadratic segment
     */
    void addQuadraticSegment(const Point& cp, const Point& q);

    /**
     * @brief Add a cubic spline segment
     * @param fp First control point
     * @param sp Second control point
     * @param q Second knot
     */
    void addCubicSegment(const Point& fp, const Point& sp, const Point& q);

    /**
     * @brief Add a cubic spline segment
     * @param fVelocity Velocity vector at the first knot
     * @param sVelocity Velocity vector at the second knot
     * @param q Second knot
     */
    void addCubicSegment(const MathVect3& fVelocity, const MathVect3 sVelocity, const Point& q);

    /**
     * @brief Set the first knot of the spline
     * @param p The new first knot
     */
    void setFirstKnot(const Point& p);

    /**
     * @brief Get the first knot
     * @return The first knot
     */
    [[maybe_unused]] const Point& getFirstKnot() const;

    /**
     * @brief Get the last knot
     * @return The last knot
     */
    const Point& getLastKnot() const;

    /**
     * @brief Get a reference to the vector containing the segments
     * @return The segments
     */
    const std::vector<PartialSplineSegment>& getSegments() const;

    /**
     * @brief Compute the smallest box containing the spline
     * @return The bounding box
     */
    Rectangle<double> getBoundingBox() const;

    /**
     * @brief Convert the spline to points and fill the given vector
     * @param points Vector of points to fill
     */
    void toPoints(std::vector<Point>& points) const;

    /**
     * @brief Get the number of spline segments
     * @return The number of spline segments
     */
    size_t size() const;

    /**
     * @brief If n < size(), resize the spline to n segments
     * @param n Number of segments to keep
     */
    void resize(size_t n);

    /**
     * @brief Move (translate) the spline
     * @param dx Shift in the x coordinate
     * @param dy Shift in the y coordinate
     */
    void move(double dx, double dy);

    using Parameter = std::pair<size_t, double>;

    /**
     * @brief
     */
    std::vector<Parameter> intersectWithRectangle(const Rectangle<double>& rectangle) const;

    /**
     * @brief
     */
    Point getPoint(Parameter parameter) const;

    [[maybe_unused]] void debugPrint() const;

private:
    /**
     * @brief The initial point of the spline
     */
    Point firstKnot;
    std::vector<PartialSplineSegment> segments;

    /**
     * Static material to generate splines:
     *
     *  ** Catmull-Rom interpolation **
     */
public:
    /**
     * @brief Compute the centripetal Catmull-Rom interpolation of a sequence of input points
     * @param points Vector containing the input points
     * @return The interpolating spline
     */
    static Spline getCentripetalCatmullRomInterpolation(const std::vector<Point>& points);

private:
    /**
     * @brief A helper class for Catmull-Rom interpolation
     */
    class CatmullRomComputer;

    /**
     * ** Schneider approximation **
     */
public:
    /**
     * @brief Compute a spline approximation of the input points using Schneider's algorithm
     * @param points Vector containing the input points
     * @return The approximating spline
     */
    static Spline getSchneiderApproximation(const std::vector<Point>& points);

private:
    /**
     * @brief A helper class for Schneider approximation
     */
    class SchneiderApproximater;
};

/**
 * @brief A helper class for Catmull-Rom interpolation
 *
 * It computes the tangent vector m to the (centripetal) Catmull-Rom spline at a point p[i].
 * Its input data consist of the vectors p[i-1]p[i] and p[i]p[i+1].
 *
 * Every time i is incremented, the vector p[i-1]p[i] is overwritten by p[i+1]p[i+2] using addStep(),
 * and the tangent vector m is recomputed.
 */
class Spline::CatmullRomComputer {
public:
    /**
     * @brief Create a CatmullRomComputer and feed it its first two vectors
     * @param u The first vector
     * @param v The second vector
     */
    CatmullRomComputer(const MathVect3& u, const MathVect3& v);

    /**
     * @brief Overwrite the oldest vector in the CatmullRomComputer's buffer and update the output m
     * @param u The new vector
     */
    void addStep(const MathVect3& u);

    /**
     * @brief Get the most recent vector from the buffer diff
     * @return A reference to the most recent vector
     */
    const MathVect3& getLastVector();

    /**
     * @brief The latest tangent vector computed
     */
    MathVect3 m;
    /**
     * @brief Square root of the norm of the oldest vector in the buffer diff
     */
    double t01;
    /**
     * @brief Square root of the norm of the newest vector in the buffer diff
     */
    double t12;

private:
    /**
     * @brief Tiny "circular buffer" containing the two last fed vectors
     */
    MathVect3 diff[2];
    /**
     * @brief Reading head of the buffer diff
     */
    unsigned int head;
};

class Spline::SchneiderApproximater {
public:
    SchneiderApproximater(const std::vector<Point>& points);

    /**
     * @brief Get the resulting spline
     * @return The spline the algorithm found
     */
    Spline getSpline();

    /** Statistics purposes. Remove before merge **/
    [[maybe_unused]] void printStats();
    static size_t totalNbSegments;
    static size_t totalNbPoints;
    /**********************************************/

private:
    /**
     * TODO Move this to wherever it fits
     */
    static inline bool fuzzyVanish(double d) { return std::abs(d) < MathVect3::EPSILON; }

    /**
     * @brief Compute (heuristically) a unit (or zero) tangent vector at the given point
     * @param it Iterator to the point of interest
     * @return A heuristically determined tangent vector at *it
     */
    MathVect3 getMiddleTangent(std::vector<Point>::const_iterator it);

    /**
     * @brief Apply Schneider's algorithm to find a spline approximating a sub-interval of the data points
     * @param lowerIndex Index of the first endpoint
     * @param firstTangentVector A prescribed unit (or zero) tangent vector at the first endpoint
     * @param secondTangentVector A prescribed unit (or zero) tangent vector at the second endpoint
     * @param upperIndex Index of the second endpoint
     */
    void fitCubic(size_t lowerIndex, const MathVect3& firstTangentVector, const MathVect3& secondTangentVector,
                  size_t upperIndex);

    /**
     * @brief Compute the standard (i.e. using chord length) parametrization of a sub-interval of data points
     * @param lowerIndex Index of the first endpoint
     * @param upperIndex Index of the second endpoint
     * @return The standard parametrization
     */
    std::vector<double> getStandardParametrization(size_t lowerIndex, size_t upperIndex);

    /**
     * @brief Helper class for the least squares method
     */
    class SingleSegmentFitter;

    /**
     * @brief Reference to the data points
     */
    const std::vector<Point>& points;

    /**
     * @brief Resulting spline
     */
    Spline spline;

    /**
     * @brief Cumulated lengths of the segments formed by the data points
     */
    std::vector<double> chordLength;

    /**
     * @brief Square of the maximal distance between the data points and the fitted curve
     */
    static constexpr double ERROR = 0.5;
    /**
     * Schneider ITERATION_ERROR = 4 * ERROR
     * Krita ITERATION_ERROR = ERROR * ERROR
     * TODO: Try different things out
     * One thing is sure: ITERATION_ERROR must be larger to ERROR
     *
     * @brief Square of the maximal error under which we try reparametrizing
     */
    static constexpr double ITERATION_ERROR = 4 * ERROR;

    /**
     * @brief Maximal number of reparametrization attempts
     */
    static constexpr size_t NUMBER_OF_ITERATIONS = 4;
};

/**
 * @brief Helper class for fitting a single spline segment with prescribed endpoints and tangents at the endpoints.
 * Uses the least squares method.
 */
class Spline::SchneiderApproximater::SingleSegmentFitter {
public:
    /**
     * @brief Create the SingleSegmentFitter
     * @param fTgt Unit tangent vector at the first endpoint
     * @param sTgt Unit tangent vector at the second endpoint
     * @param pointsBegin Iterator to the first endpoint
     * @param pointsLast Iterator to the second endpoint
     * @param parametrization A parametrization of the path following the points of the range {pointsBegin, pointsLast}
     */
    SingleSegmentFitter(const MathVect3& fTgt, const MathVect3& sTgt, std::vector<Point>::const_iterator pointsBegin,
                        std::vector<Point>::const_iterator pointsLast, std::vector<double> parametrization);

    /**
     * @brief Applies the least squares method to find the best fit
     */
    void findBestCubicSegment();

    /**
     * @brief Compute the error vector for each point (and populate errors) and find the point with worst error.
     * @return The squared norm of the longest error vector
     */
    double computeMaxError();

    /**
     * @brief Use the Raphson-Newton method to improve the parametrization (and thus reduce the errors)
     * @return True if the new parametrization is valid, False otherwise
     */
    bool reparametrize();

    /**
     * @brief Get the best fitted velocity vector of the first endpoint
     * @return The velocity vector
     */
    MathVect3 getFirstVelocity() const;

    /**
     * @brief Get the best fitted velocity vector of the second endpoint
     * @return The velocity vector
     */
    MathVect3 getSecondVelocity() const;

    /**
     * @brief Get an iterator to the point with longest error vector
     * @return The iterator
     */
    std::vector<Point>::const_iterator getWorstPoint() const;

private:
    /**
     * @brief Unit (or zero) tangent vector at the first endpoint
     */
    const MathVect3 fTgt;
    /**
     * @brief Unit (or zero) tangent vector at the second endpoint
     */
    const MathVect3 sTgt;
    /**
     * @brief Vector from the first endpoint to the second
     */
    const MathVect3 diff;

    /**
     * Precomputed scalar products
     */
    /**
     * @brief Scalar product of fTgt and sTgt
     */
    const double sp_fTgt_sTgt;
    /**
     * @brief Scalar product of fTgt and diff
     */
    const double sp_fTgt_diff;
    /**
     * @brief Scalar product of sTgt and diff
     */
    const double sp_sTgt_diff;

    /**
     * @brief Squared norm of diff
     */
    const double squaredNormDiff;

    /**
     * @brief Whether fTgt is the 0 vector or not
     */
    const bool fTgtZero;
    /**
     * @brief Whether sTgt is the 0 vector or not
     */
    const bool sTgtZero;


    /**
     * @brief Iterator to the first endpoint
     */
    const std::vector<Point>::const_iterator pointsBegin;

    /**
     * @brief Iterator to the successor of the second endpoint
     */
    const std::vector<Point>::const_iterator pointsEnd;

    /**
     * @brief Iterator to the point with longest error vector
     */
    std::vector<Point>::const_iterator worstPoint;

    /**
     * In the following comments, u -> Q(u) will be the spline segment best fitting the data points
     */
    /**
     * @brief First velocity vector of the best fit: Q'(0)
     */
    MathVect3 fVelocity;
    /**
     * @brief Second velocity vector of the best fit: -Q'(1)
     */
    MathVect3 sVelocity;

    /**
     * @brief Parametrization of the path {pointsBegin, std::previous(pointsEnd)}
     */
    std::vector<double> parametrization;

    /**
     * @brief Error vectors
     * Contain the vectors errors[i] = Q(parametrization[i]) - pointsBegin[i]
     */
    std::vector<MathVect3> errors;
};
