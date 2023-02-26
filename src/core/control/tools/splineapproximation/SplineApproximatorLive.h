/**
 * Xournal++
 *
 * A live version of Schneider's algorithm
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
 *
 * It has been heavily modified
 */

#pragma once

#include <memory>  // for shared_ptr
#include <vector>  // for vector

#include "model/MathVect.h"       // for MathVect3
#include "model/Point.h"          // for Point
#include "model/SplineSegment.h"  // for SplineSegment
#include "model/path/Spline.h"    // for Spline

#include "SplineApproximator.h"

#ifdef SHOW_APPROX_STATS
#include <chrono>
#endif

class SplineApproximator::Live {
public:
    virtual ~Live() = default;

    Live(std::shared_ptr<Spline> ptr);

    /**
     * @brief Feed a point to the live approximator
     * @param p the point
     * @return True if a spline segment fitting all the points was found. False otherwise.
     */
    bool feedPoint(const Point& p);
    bool finalize();

    double totalLength = 0.0;
    MathVect3 firstTangentVector;
    SplineSegment lastDefinitiveSegment;
    SplineSegment liveSegment;
    size_t dataCount = 0;

    /**
     * Handmade circular buffer.
     * The live approximator is 5% more efficient with this rather than with a util/CircularBuffer
     */
    Point pts[4];
    Point* P0 = pts;
    Point* P1 = pts + 1;
    Point* P2 = pts + 2;
    Point* P3 = pts + 3;

    Point firstKnot;

    std::vector<BufferData> buffer;

    /**
     * @brief Get the resulting spline
     * @return The spline the algorithm found
     */
    Spline getSpline();

#ifdef SHOW_APPROX_STATS
    void printStats();
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
    void pushToCircularBuffer(const Point& p);
    void processFirstEvents();
    void startNewSegment(const MathVect3& lastTangentVector);

    /**
     * @brief Resulting spline
     */
    std::shared_ptr<Spline> spline;
};
