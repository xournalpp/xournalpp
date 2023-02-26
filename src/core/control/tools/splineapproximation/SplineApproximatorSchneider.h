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

#include <vector>  // for vector, vector<>::iterator

#include "model/MathVect.h"     // for MathVect3
#include "model/path/Spline.h"  // for Spline

#include "SplineApproximator.h"

class Point;

#ifdef SHOW_APPROX_STATS
#include <chrono>
#endif

class SplineApproximator::Schneider {
public:
    Schneider(const std::vector<Point>& points);

    void run();

    /**
     * @brief Get the resulting spline
     * @return The spline the algorithm found
     */
    Spline getSpline();

#ifdef SHOW_APPROX_STATS
    /** Statistics purposes. Remove before merge **/
    [[maybe_unused]] void printStats();
    static size_t totalNbSegments;
    static size_t totalNbPoints;
    static std::chrono::microseconds timeSpent;
    std::chrono::high_resolution_clock::time_point t1;
    void inline startTimer() { t1 = std::chrono::high_resolution_clock::now(); }
    void inline stopTimer() {
        auto t2 = std::chrono::high_resolution_clock::now();
        timeSpent += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
    }
#endif

private:
    /**
     * @brief Compute (heuristically) a unit (or zero) tangent vector at the given point
     * @param it Iterator to the point of interest
     * @return A heuristically determined tangent vector at *it
     */
    MathVect3 getMiddleTangent(std::vector<BufferData>::iterator it);

    /**
     * @brief Apply Schneider's algorithm to find a spline approximating a sub-interval of the data points
     * @param lowerIndex Index of the first endpoint
     * @param firstTangentVector A prescribed unit (or zero) tangent vector at the first endpoint
     * @param secondTangentVector A prescribed unit (or zero) tangent vector at the second endpoint
     * @param upperIndex Index of the second endpoint
     */
    void fitCubic(std::vector<BufferData>::iterator beginData, const MathVect3& firstTangentVector,
                  const MathVect3& secondTangentVector, std::vector<BufferData>::iterator lastData, double ref);

    /**
     * @brief Reference to the data points
     */
    const std::vector<Point>& points;

    /**
     * @brief
     */
    std::vector<BufferData> buffer;

    /**
     * @brief Resulting spline
     */
    Spline spline;
};
