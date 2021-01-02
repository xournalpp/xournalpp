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
#include "SplineSegment.h"

/**
 * @brief A class to handle splines
 * A spline is only assumed to be continuous.
 */
class Spline {
public:
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

    void addLineSegment(const Point& q);
    void addQuadraticSegment(const Point& cp, const Point& q);
    void addCubicSegment(const Point& fp, const Point& sp, const Point& q);
    void addCubicSegment(const MathVect3& fVelocity, const MathVect3 sVelocity, const Point& q);

    Point getFirstKnot() const;
    Point getLastKnot() const;
    const std::vector<PartialSplineSegment>& getSegments() const;

    /**
     * @brief Convert the spline to a list of points.
     * @return A point list which represents the spline.
     */
    std::list<Point> toPointSequence() const;

    /**
     * @brief Get the number of spline segments
     * @return The number of spline segments
     */
    size_t size() const;

    void simplify();

    [[maybe_unused]] void debugPrint();

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
    Spline getSpline();

private:
    static inline bool fuzzyVanish(double d) { return std::abs(d) < MathVect3::EPSILON; }
    
    MathVect3 getMiddleTangent(std::vector<Point>::const_iterator it);
    void fitCubic(size_t lowerIndex, const MathVect3& firstTangentVector, const MathVect3& secondTangentVector,
                  size_t upperIndex);
    
    std::vector<double> getStandardParametrization(size_t lowerIndex, size_t upperIndex);

    /**
     * @brief Helper class for the least squares method
     */
    class SingleSegmentFitter;

    const std::vector<Point>& points;
    Spline spline;

    std::vector<double> chordLength;

    /**
     * Square of the maximal distance between the data points and the fitted curve
     */
    static constexpr double ERROR = 0.5;
    /**
     * Schneider ITERATION_ERROR = 4 * ERROR
     * Krita ITERATION_ERROR = ERROR * ERROR
     * TODO: Try different things out
     */
    static constexpr double ITERATION_ERROR = 4 * ERROR;
    
    static constexpr size_t NUMBER_OF_ITERATIONS = 4;
};

class Spline::SchneiderApproximater::SingleSegmentFitter {
public:
    SingleSegmentFitter(const MathVect3& fTgt, const MathVect3& sTgt, 
                        std::vector<double> parametrization,
                        std::vector<Point>::const_iterator pointsBegin,
                        std::vector<Point>::const_iterator pointsLast);
    
    void findBestCubicSegment();
    
    double computeMaxError();
    
    MathVect3 getFirstVelocity() const;
    MathVect3 getSecondVelocity() const;
    
    bool reparametrize();
    
    std::vector<Point>::const_iterator getWorstPoint() const;
    
private:
    const MathVect3 fTgt;
    const MathVect3 sTgt;
    const MathVect3 diff;
    
    /**
     * Precomputed scalar products
     */
    const double sp_fTgt_sTgt;
    const double sp_fTgt_diff;
    const double sp_sTgt_diff;

    const bool fTgtZero;
    const bool sTgtZero;

    /**
     * Parametrization and associated points
     */
    std::vector<double> parametrization;
    std::vector<MathVect3> errors;

    /**
     * Iterators to the data
     */
    std::vector<Point>::const_iterator pointsBegin;
    std::vector<Point>::const_iterator pointsEnd;
    
    std::vector<Point>::const_iterator worstPoint;
    
    /**
     * Result
     */
    MathVect3 fVelocity;
    MathVect3 sVelocity;
};
