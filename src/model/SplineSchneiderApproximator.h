/**
 * Xournal++
 *
 * An implementation of Schneider's algorithm
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
/**
 * The algorithms implemented here come from
 *
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */

#pragma once

#include "Spline.h"

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

    /**
     * @brief Factor applied to the error in the pressure direction
     * The bigger, the less the pressure value can differ from that of the original points
     */
    static constexpr double PRESSURE_FAITHFULNESS = 3.0;
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
