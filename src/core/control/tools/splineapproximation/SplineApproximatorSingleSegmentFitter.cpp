#include "SplineApproximatorSingleSegmentFitter.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>

#include <glib.h>  // for g_warning

SplineApproximator::SingleSegmentFitter::SingleSegmentFitter(const MathVect3& fTgt, const MathVect3& sTgt,
                                                             std::vector<BufferData>::iterator bufferBegin,
                                                             std::vector<BufferData>::iterator bufferLast, double ref):
        bufferBegin(bufferBegin),
        bufferLast(bufferLast),
        fTgt(fTgt),
        sTgt(sTgt),
        diff(this->bufferLast->step),
        sp_fTgt_sTgt(MathVect3::scalarProduct(this->fTgt, this->sTgt)),
        sp_fTgt_diff(MathVect3::scalarProduct(this->fTgt, this->diff)),
        sp_sTgt_diff(MathVect3::scalarProduct(this->sTgt, this->diff)),
        squaredNormDiff(MathVect3::scalarProduct(this->diff, this->diff)),
        fTgtZero(this->fTgt.dx == 0.0 && this->fTgt.dy == 0.0),   // No need to fuzzy compare (see fTgt.normalize())
        sTgtZero(this->sTgt.dx == 0.0 && this->sTgt.dy == 0.0) {  // No need to fuzzy compare (see sTgt.normalize())

    /**
     * The parametrization is shorter than the buffer by 1
     * The parameter of the last point is always 1 so no need to store it.
     *
     * The BufferData *bufferLast is only used here to set the variable diff up and to get the total chordLength
     */
    this->parametrization.reserve(static_cast<size_t>(std::distance(this->bufferBegin, this->bufferLast)));
    double max = this->bufferLast->chordLength - ref;
    std::transform(this->bufferBegin, this->bufferLast, std::back_inserter(this->parametrization),
                   [max, ref](const BufferData& data) { return (data.chordLength - ref) / max; });
}

bool SplineApproximator::SingleSegmentFitter::findBestCubicSegment() {
    /**
     * Find the best possible (single) spline segment fitting the data points, with prescribed endpoints and with
     * initial and final velocity vectors proportional to the provided tangent vectors
     *
     * The fitted parameters are the norms of the two velocity vectors.
     */
    double lastError = ITERATION_ERROR;

    for (size_t i = 0; i < NUMBER_OF_ITERATIONS; i++) {
        auto norms = this->getBestFitNorms();

        this->fVelocity = norms.first * this->fTgt;
        this->sVelocity = norms.second * this->sTgt;

        double maxError = this->computeMaxError();
        if (maxError < ERROR) {
            return true;
        }
        if (maxError >= lastError || !this->reparametrize()) {
            /**
             * The error is either to big or getting bigger
             * or
             * the reparametrization failed
             */
            return false;
        }
        lastError = maxError;
    }
    return false;
}

auto SplineApproximator::SingleSegmentFitter::computeSystem() -> LinearSystem {
    /**
     * Fill the matrices. Compared to Schneider's original code
     *      https://github.com/erich666/GraphicsGems/blob/master/gems/FitCurves.c
     * this has been optimized by:
     *  * computing most scalar products only once
     *  * multiplying by the scale factors after taking the scalar products
     *  * expressing the points' coordinates relatively to the first point so the computation of X has less
     * terms
     */
    auto res = std::transform_reduce(
            this->bufferBegin, this->bufferLast, this->parametrization.begin(), LinearSystem(0.0, 0.0, 0.0, 0.0, 0.0),
            std::plus<>(), [this](const BufferData& d, const double& u) -> LinearSystem {
                double t = 1 - u;
                double b = 3 * u * t;
                double b1 = b * t;
                double b2 = b * u;
                double b23 = b2 + u * u * u;
                return LinearSystem(b1 * b1, b1 * b2, b2 * b2,
                                    b1 * (d.scalarProductWithFirstTangent - (b23 * this->sp_fTgt_diff)),
                                    b2 * (MathVect3::scalarProduct(this->sTgt, d.step) - (b23 * this->sp_sTgt_diff)));
            });

    res.c00 = (this->fTgtZero ? 0.0 : res.c00);
    res.c01 *= this->sp_fTgt_sTgt;
    res.c11 = (this->sTgtZero ? 0.0 : res.c11);

    return res;
}

auto SplineApproximator::SingleSegmentFitter::getBestFitNorms() -> std::pair<double, double> {
    auto norms = this->computeSystem().solution();

    if (norms.first <= 0.0 || norms.second <= 0.0 ||
        norms.first * this->sp_fTgt_diff - norms.second * this->sp_sTgt_diff > squaredNormDiff) {
        // Fallback to the Wu-Barsky heuristic
        norms.first = norms.second = std::sqrt(squaredNormDiff) / 3.0;
    }
    return norms;
}

auto SplineApproximator::SingleSegmentFitter::LinearSystem::solution() const -> std::pair<double, double> {
    std::pair<double, double> solution;
    double detC = determinant();
    if (!fuzzyVanish(detC)) {
        // Use Kramer's rule to solve the system
        solution.first = (c11 * x0 - c01 * x1) / detC;
        solution.second = (c00 * x1 - c01 * x0) / detC;
    } else {
        // The system is under-determined. Try assuming first == second.
        double c = c00 + c01;
        if (!fuzzyVanish(c)) {
            solution.first = x0 / c;
        } else {
            c = c01 + c11;
            if (!fuzzyVanish(c)) {
                solution.first = x1 / c;
            } else {
                solution.first = 0.0;
            }
        }
        solution.second = solution.first;
    }
    return solution;
}

double SplineApproximator::SingleSegmentFitter::computeMaxError() {
    double maxDistance = 0.0;

    this->errors.clear();
    this->errors.reserve(this->parametrization.size());

    auto itParam = this->parametrization.begin();
    for (auto itData = this->bufferBegin; itData != this->bufferLast; ++itData, ++itParam) {
        double u = *itParam;
        double t = 1 - u;
        double ut3 = u * t * 3;
        /**
         * Compute the error vector. To simplify the formulae, the origin is set to firstKnot.
         */
        MathVect3 error = (t * ut3) * this->fVelocity + (ut3 * u) * this->sVelocity +
                          ((1 + 2 * t) * u * u) * this->diff - itData->step;

        double pressure = error.dz;
        error.dz *= PRESSURE_FAITHFULNESS;

        if (double dist = MathVect3::scalarProduct(error, error); dist > maxDistance) {
            maxDistance = dist;
            this->worstPoint = itData;
        }

        error.dz = pressure;
        this->errors.push_back(error);
    }
    return maxDistance;
}

bool SplineApproximator::SingleSegmentFitter::reparametrize() {
    auto errorIt = this->errors.cbegin();
    for (double& u: this->parametrization) {
        double t = 1 - u;
        double threeUMinusTwo = 3.0 * u - 2.0;
        double oneMinusThreeU = -1.0 - threeUMinusTwo;
        double twoU = 2.0 * u;

        /**
         * Compute the derivatives for Newton-Raphson iteration
         */
        MathVect3 thirdQprimeOfU = t * oneMinusThreeU * this->fVelocity + (twoU * t) * this->diff +
                                   (-u * threeUMinusTwo) * this->sVelocity;
        MathVect3 sixthQsecondOfU =
                threeUMinusTwo * this->fVelocity + (1 - twoU) * this->diff + oneMinusThreeU * this->sVelocity;

        double denominator = 3.0 * MathVect3::scalarProduct(thirdQprimeOfU, thirdQprimeOfU) +
                             2.0 * MathVect3::scalarProduct(*errorIt, sixthQsecondOfU);

        if (fuzzyVanish(denominator)) {  // denominator == 0.0) {
            g_warning("SplineApproximator::SingleSegmentFitter::reparametrize: zero denominator");
            ++errorIt;
            continue;
        }
        double numerator = MathVect3::scalarProduct(*errorIt, thirdQprimeOfU);
        u -= numerator / denominator;
        ++errorIt;
    }

    // The new parametrization is invalid if it is not sorted
    return std::is_sorted(this->parametrization.cbegin(), this->parametrization.cend());
}

MathVect3 SplineApproximator::SingleSegmentFitter::getFirstVelocity() const { return this->fVelocity; }

MathVect3 SplineApproximator::SingleSegmentFitter::getSecondVelocity() const { return this->sVelocity; }

std::vector<SplineApproximator::BufferData>::iterator SplineApproximator::SingleSegmentFitter::getWorstPoint() const {
    return this->worstPoint;
}
