/**
 * Xournal++
 *
 * Helper class for fitting a single spline segment
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
/**
 * The algorithm implemented here has been inspired by
 *
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */
#pragma once

#include <utility>  // for pair
#include <vector>   // for vector

#include "model/MathVect.h"

#include "SplineApproximator.h"

/**
 * @brief Helper class for fitting a single spline segment with prescribed endpoints and tangents at the endpoints.
 * Uses the least squares method.
 */
class SplineApproximator::SingleSegmentFitter {
public:
    /**
     * @brief Create the SingleSegmentFitter
     * @param fTgt Unit tangent vector at the first endpoint
     * @param sTgt Unit tangent vector at the second endpoint
     * @param bufferBegin Iterator to the first BufferData
     * @param bufferLast Iterator to the last BufferData (not the end iterator)
     */
    SingleSegmentFitter(const MathVect3& fTgt, const MathVect3& sTgt, std::vector<BufferData>::iterator bufferBegin,
                        std::vector<BufferData>::iterator bufferLast, double ref = 0.0);

    /**
     * @brief Applies the least squares method to find the best fit
     */
    bool findBestCubicSegment();

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
     * @brief Get an iterator to the BufferData with longest error vector
     * @return The iterator
     */
    std::vector<BufferData>::iterator getWorstPoint() const;

private:
    /**
     * @brief Use the least square method to get the best fitting norms of the velocity vectors
     */
    std::pair<double, double> getBestFitNorms();

    struct LinearSystem {
        LinearSystem(double c00, double c01, double c11, double x0, double x1):
                c00(c00), c01(c01), c11(c11), x0(x0), x1(x1) {}
        // Matrix coefficient
        double c00;
        double c01;
        double c11;

        // Vector coefficient
        double x0;
        double x1;

        inline double determinant() const { return c00 * c11 - c01 * c01; }
        LinearSystem operator+(const LinearSystem& other) const {
            return {c00 + other.c00, c01 + other.c01, c11 + other.c11, x0 + other.x0, x1 + other.x1};
        }

        std::pair<double, double> solution() const;
    };

    /**
     * @brief Get the system (matrix + vector) to be solved for the least square method.
     */
    LinearSystem computeSystem();

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
     * @brief Iterator to the front of the buffer
     */
    const std::vector<BufferData>::iterator bufferBegin;

    /**
     * @brief Iterator to the back of the buffer (not the end!)
     */
    const std::vector<BufferData>::iterator bufferLast;

    /**
     * @brief Iterator to the BufferData with longest error vector
     */
    std::vector<BufferData>::iterator worstPoint;

    /**
     * @brief Unit (or zero) tangent vector at the first endpoint
     */
    const MathVect3& fTgt;

    /**
     * @brief Unit (or zero) tangent vector at the second endpoint
     */
    const MathVect3& sTgt;

    /**
     * @brief Vector from the first endpoint to the second
     */
    const MathVect3& diff;

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
     * @brief Parametrization of the path
     */
    std::vector<double> parametrization;

    /**
     * @brief Error vectors
     * Contain the vectors errors[i] = Q(parametrization[i]) - points[i]
     */
    std::vector<MathVect3> errors;
};
