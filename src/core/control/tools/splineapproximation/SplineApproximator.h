/*
 * Xournal++
 *
 * Handles input of strokes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cmath>

#include "model/MathVect.h"

#define SHOW_APPROX_STATS
#ifdef SHOW_APPROX_STATS
#define APPROX_STATS(f) f
#else
#define APPROX_STATS(f)
#endif

namespace SplineApproximator {

enum class Type { NONE, LIVE, POST };

constexpr bool isValid(Type t) { return t == Type::NONE || t == Type::LIVE || t == Type::POST; }


/**
 * @brief Data structure for Schneider's approximation.
 *
 * It is constructed relatively to a fixed first knot O.
 * It corresponds to another entry point P
 */
struct BufferData {
    BufferData(const MathVect3& step, double length, double sp):
            step(step), chordLength(length), scalarProductWithFirstTangent(sp) {}
    BufferData(const Point& p, const Point& q, double length, const MathVect3& firstTangentVector):
            step(p, q),
            chordLength(length),
            scalarProductWithFirstTangent(MathVect3::scalarProduct(step, firstTangentVector)) {}

    /**
     * @brief Vector OP
     */
    MathVect3 step;

    /**
     * @brief Length of the piecewise linear path going from O to P
     */
    double chordLength;

    /**
     * @brief Scalar product of a fixed tangent vector (at O) with the vector OP.
     */
    double scalarProductWithFirstTangent;
};

class SingleSegmentFitter;
class Live;
class Schneider;


/**
 * TODO Move this to wherever it fits
 */
inline bool fuzzyVanish(double d) { return std::abs(d) < MathVect3::EPSILON; }


/*********** Constants ***********/
/**
 * @brief Square of the maximal distance between the data points and the fitted curve
 */
static constexpr double ERROR = 0.5;
/**
 * @brief Square of the maximal error under which we try reparametrizing
 *
 * ITERATION_ERROR must be larger than ERROR
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

}  // namespace SplineApproximator
