/*
 * Xournal++
 *
 * A solver of polynomial equations (up to degree 6)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <vector>

#include "Interval.h"

// #define POLY_DEBUG
#ifdef POLY_DEBUG
#include <stdio.h>
#endif

namespace PolynomialSolver {

constexpr double FUZZY_ZERO = 1e-20;  // A to big value will result in numerical instability (e.g. 1e-10 is to big...)
constexpr double MAX_ERROR = 1e-6;
constexpr double SQRT3 = 1.7320508075688772935274463415058723669428;

/**
 * @brief Compute the roots (in [min, max]) of the polynomial equation a * t^2 + 2 * b * t + c
 *
 * Warning: double roots are (purposefully) ignored.
 * If the polynomial factors as a * (t - u)^2, the returned vector will be empty
 * [This means we only get roots at which the sign of the polynomial function changes]
 *
 * Warning: Very small values for the leading coefficient a will result in very high numerical errors
 *
 * @param a Quadratic coefficient
 * @param b Half of linear coefficient
 * @param c Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop): 3,998,731 microseconds for 10,000,000 iterations
 */
std::vector<double> rootsOfQuadratic(double a, double b, double c, double min, double max);

/**
 * @brief Compute the roots (in [min, max]) of the polynomial equation a*t^3 + 3*b*t^2 + 3*c*t + d
 *
 * Warning: double roots are (purposefully) ignored.
 * If the polynomial factors as a * (t - u) * (t - v)^2, the returned vector will only contain u.
 * [This means we only get roots at which the sign of the polynomial function changes]
 *
 * A triple root will be returned (without multiplicities).
 *
 * Warning: Very small values for the leading coefficient a will result in very high numerical errors
 *
 * @param a Cubic coefficient
 * @param b Third of quadratic coefficient
 * @param c Third of linear coefficient
 * @param d Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop): 6,009,103 microseconds for 10,000,000 iterations
 */
std::vector<double> rootsOfCubic(double a, double b, double c, double d, double min, double max);

/**
 * @brief Find roots (in [min, max]) of a*t^4 + 4*b*t^3 + 6*c*t^2 + 4*d*t + e
 *
 * Warning: double and quadruple roots are (purposefully) ignored.
 * [This means we only get roots at which the sign of the polynomial function changes]
 * Beware: due to numerical errors, in some cases, a double root may still be returned as twice very close values
 *
 * A triple root will be returned (without multiplicities).
 *
 * Warning: Very small values for the leading coefficient a will result in very high numerical errors
 *
 * @param a Quartic coefficient
 * @param b Fourth of cubic coefficient
 * @param c Sixth of quadratic coefficient
 * @param d Fourth of linear coefficient
 * @param e Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop): 7,400,889 microseconds for 10,000,000 iterations
 */
std::vector<double> rootsOfQuartic(double a, double b, double c, double d, double e, double min, double max);

/**
 * @brief Remove all entries below min or above max from the sorted vector input and return the resulting vector
 * @param input A sorted input vector
 * @param min Lower bound
 * @param max Upper bound
 * @return A sorted vector containing those elements of input between min and max.
 */
std::vector<double> truncate(const std::vector<double>& input, double min, double max);

/**
 * @brief Returns a sortec vector containing the values of r1 and r2 that are between min and max. Assume r1 < r2.
 * @param r1 First value
 * @param r2 Second value
 * @param min Lower bound
 * @param max Upper bound
 * @return A sorted vector containing those elements of {r1, r2} between min and max.
 *
 * This is equivalent to but much faster than truncate({r1, r2}, min, max)
 */
std::vector<double> truncate2(double r1, double r2, double min, double max);

#ifdef POLY_DEBUG
// Test
[[maybe_unused]] void benchmark();
#endif

/**************
 * Polynomial *
 **************/
template <size_t N>
class Polynomial {
public:
    Polynomial() = default;

    [[maybe_unused]] double evaluate(double t) const;

    [[maybe_unused]] std::pair<double, double> getMinimum(double min, double max);

    [[maybe_unused]] Polynomial<N - 1> getDerivative() const;

    std::array<double, N + 1> coeff;
};

template <size_t N>
double Polynomial<N>::evaluate(double t) const {
    return std::accumulate(this->coeff.begin(), this->coeff.end(), 0.0, [t](double r, double s) {
        r *= t;
        r += s;
        return r;
    });
}

template <size_t N>
Polynomial<N - 1> Polynomial<N>::getDerivative() const {
    Polynomial<N - 1> derivative;
    double n = N + 1;
    std::transform(this->coeff.begin(), this->coeff.end() - 1, derivative.coeff.begin(),
                   [&n](double c) { return c * (--n); });
    return derivative;
}


/********************
 * PolynomialSolver *
 ********************/
/**
 * Benchmark (on a modest laptop):
 *  * 1,000,000 resolutions of degree 6 polynomials in 4,008,539 µs (~4 µs per resolution)
 *  * 1,000,000 resolutions of degree 5 polynomials in 3,076,094 µs (~3 µs per resolution)
 *
 * (Nb: replacing the C-type arrays with std::array and the loops using std::algorithm is much slower:
 *   degree 6: 7,249,618 µs)
 */
template <size_t N>
class PolynomialSolver {
    static_assert(N > 4, "Error: PolynomialSolver::PolynomialSolver<N> can only be used with N > 4. Use "
                         "PolynomialSolver::rootsOfQuadratic, PolynomialSolver::rootsOfCubic or "
                         "PolynomialSolver::rootsOfQuartic instead.");

public:
    /**
     * @brief Create a solver for the polynomial whose coefficients are given.
     * The coefficient of highest degree comes first:
     * PolynomialSolver({a_n, ..., a_0}) will solve the polynomial P(t) = a_n * t^n + .... + a_0
     */
    PolynomialSolver(const Polynomial<N>& poly) {
        // convert higher degree coeff first to lower degree coeff first
        std::copy(poly.coeff.rbegin(), poly.coeff.rend(), derivatives[0]);

        // Compute the coefficients of the derivatives
        for (double *it = derivatives[0] + 1, *end = it + N; it != derivatives[N - 2] + 1; it += N + 1, end += N) {
            double mult = 1.0;
            for (double *it2 = it, *it3 = it + N; it2 != end; ++it2, ++it3) {
                *it3 = mult * *it2;
                ++mult;
            }
        }
    }

    /**
     * @brief Compute the roots of the polynomial that lie between min and max.
     * @param min Lower bound for roots of interest
     * @param max Upper bound for roots of interest
     * @return A vector containing the roots.
     */
    std::vector<double> findRoots(double min, double max);

private:
    /**
     * @brief Compute P^{(n)}(t) (the value of the n-th derivative at t)
     * @param n Rank of the derivative
     * @param t Parameter at which we evaluate
     * @return The value P^{(n)}(t)
     */
    double evaluateNthDerivative(size_t n, double t);

    /**
     * @brief Array containing the coefficients of the iterated derivatives P = P^{(0)}, P^{(1)}, ..., P^{(N-2)} of the
     * target polynomial equation P. The value of derivatives[i][j] is the coefficient of order j in P^{(i)}.
     *
     * Note that derivatives[i][j] is not initialized (nor used) if j > N - i + 1 = deg(P^{(i)}).
     */
    double derivatives[N - 1][N + 1];
};

template <size_t N>
std::vector<double> PolynomialSolver<N>::findRoots(double min, double max) {


    /**
     * For the duration of the following loop, this will contain the roots (between min and max) of the iterated
     * derivatives P^{(n)}, P^{(n+1)} and P^{(n+2)} (up to a cyclic permutation).
     */
    std::vector<double> roots[3];

    /**
     * First compute the roots in (min, max) of the N-2nd, N-3rd and N-4th derivatives
     */
    double* deg2Der = derivatives[N - 2];
    double* deg3Der = derivatives[N - 3];
    double* deg4Der = derivatives[N - 4];

    roots[(N + 1) % 3] = rootsOfQuadratic(deg2Der[2], 0.5 * deg2Der[1], deg2Der[0], min, max);
    roots[N % 3] = rootsOfCubic(deg3Der[3], deg3Der[2] / 3.0, deg3Der[1] / 3.0, deg3Der[0], min, max);
    roots[(N + 2) % 3] =
            rootsOfQuartic(deg4Der[4], 0.25 * deg4Der[3], deg4Der[2] / 6.0, 0.25 * deg4Der[1], deg4Der[0], min, max);

    for (size_t n = N - 5; n < N; --n) {
        /**
         * Find the roots of Q = P^{(n)} the n-th derivative of our target polynomial equation.
         * The roots of Q', Q'' and Q''' are already computed and stored in `roots`
         */

        //  Aliases for the root vectors
        const std::vector<double>& rootsOfDerivative = roots[(n + 1) % 3];
        const std::vector<double>& rootsOfSecondDerivative = roots[(n + 2) % 3];
        std::vector<double>& rootsOfThirdDerivative = roots[n % 3];

        /**
         * First, we compute basins of attraction for Halley's method.
         * Those basins are intervals in which Q', Q'' and Q''' do not vanish (assuming Q has simple roots)
         * The boolean represents the sign of Q at the interval's bound (true = positive, false = negative)
         *
         * We start by finding intervals on which Q' does not vanish and that must contain exactly one root.
         */
        std::vector<Interval<std::pair<double, bool>>> basins;
        if (rootsOfDerivative.empty()) {
            if (bool signMin = evaluateNthDerivative(n, min) > 0.0, signMax = evaluateNthDerivative(n, max) > 0.0;
                signMin != signMax) {
                basins = {{{min, signMin}, {max, signMax}}};
            } else {
                // The function is monotonous and has same sign on endpoints: it does not have roots between min and max
                rootsOfThirdDerivative.clear();
                continue;  // Move on to the next derivative
            }
        } else {
            basins.reserve(rootsOfDerivative.size() + 1);
            double firstRoot = rootsOfDerivative.front();
            double lastSign = evaluateNthDerivative(n, firstRoot) > 0.0;
            if (bool signMin = evaluateNthDerivative(n, min) > 0.0; signMin != lastSign) {
                basins.emplace_back(std::make_pair(min, signMin), std::make_pair(firstRoot, lastSign));
            }
            for (auto it1 = rootsOfDerivative.begin(), it2 = it1 + 1; it2 != rootsOfDerivative.end(); it1 = it2++) {
                bool sign = evaluateNthDerivative(n, *it2) > 0.0;
                if (sign != lastSign) {
                    /**
                     * P(*it1) and P(*it2) have different signs (and P'(t) != 0 for t in (*it1, *it2))
                     * Keep this interval
                     */
                    basins.emplace_back(std::make_pair(*it1, lastSign), std::make_pair(*it2, sign));
                    lastSign = sign;
                }
            }
            if (bool signMax = evaluateNthDerivative(n, max) > 0.0; signMax != lastSign) {
                basins.emplace_back(std::make_pair(rootsOfDerivative.back(), lastSign), std::make_pair(max, signMax));
            }
        }
        /**
         * Shrink the basins so that P'' does not vanish
         */
        if (!rootsOfSecondDerivative.empty()) {
            auto it = rootsOfSecondDerivative.begin();
            auto end = rootsOfSecondDerivative.end();
            for (auto&& basin: basins) {
                while (it != end && basin.max.first > *it) {
                    if (basin.min.first < *it) {
                        if (bool sign = evaluateNthDerivative(n, *it) > 0.0; sign != basin.min.second) {
                            basin.max = {*it, sign};
                        } else {
                            basin.min = {*it, sign};
                        }
                    }
                    ++it;
                }
                if (it == end) {
                    break;
                }
            }
        }
        /**
         * Shrink the basins so that P''' does not vanish
         * This is necessary to avoid overshoots
         */
        if (!rootsOfThirdDerivative.empty()) {
            auto it = rootsOfThirdDerivative.begin();
            auto end = rootsOfThirdDerivative.end();
            for (auto&& basin: basins) {
                while (it != end && basin.max.first > *it) {
                    if (basin.min.first < *it) {
                        if (bool sign = evaluateNthDerivative(n, *it) > 0.0; sign != basin.min.second) {
                            basin.max = {*it, sign};
                        } else {
                            basin.min = {*it, sign};
                        }
                    }
                    ++it;
                }
                if (it == end) {
                    break;
                }
            }
            // rootsOfThirdDerivative will be overwritten with the roots of Q = P^{(n)}.
            rootsOfThirdDerivative.clear();
        }

        /**
         * Now compute the root within each basin and store them in the empty rootsOfThirdDerivative
         */
        rootsOfThirdDerivative.reserve(basins.size());

        for (auto&& basin: basins) {
            double t = 0.5 * (basin.min.first + basin.max.first);
            double value = evaluateNthDerivative(n, t);
            size_t count = 0;

            while (std::abs(value) > MAX_ERROR && count < 10) {
                count++;

#define HALLEY
#ifdef HALLEY
                /* Halley iteration */
                double derValue = evaluateNthDerivative(n + 1, t);
                double secValue = evaluateNthDerivative(n + 2, t);
                t -= value * derValue / (derValue * derValue - 0.5 * value * secValue);
                /********************/
#else
                /* Newton iteration */
                double derValue = evaluateNthDerivative(n + 1, t);
                t -= value / derValue;
                /********************/
#endif
                value = evaluateNthDerivative(n, t);
            }
            rootsOfThirdDerivative.push_back(t);

#ifdef POLY_DEBUG
            if ((t < min || t > max) || (count == 10 && std::abs(value) > MAX_ERROR)) {
                printf("\nDegree %zu -- Iterations: %zu", N - n, count);

                printf(" -- Interval: [%f ; %f)\n", basin.min.first, basin.max.first);

                printf(" -- val: [%13.11f ; %13.11f)\n", evaluateNthDerivative(n, basin.min.first),
                       evaluateNthDerivative(n, basin.max.first));

                printf("  P(t) =");
                size_t deg = N - n;
                for (double* it = derivatives[n] + N - n; it >= derivatives[n]; --it, --deg) {
                    printf(" %f*t^%zu +", *it, deg);
                }
                printf("\b\b\b\b    \n");


                printf("    ** root found t = %f ; P(t) = %13.11f\n", t, value);

                printf("    ** roots of der:\n");
                for (auto&& t: rootsOfDerivative) {
                    printf("      ** t = %f ; P''(t) = %13.11f ; P'(t) = %13.11f ; P(t) = %13.11f\n", t,
                           evaluateNthDerivative(n + 2, t), evaluateNthDerivative(n + 1, t),
                           evaluateNthDerivative(n, t));
                }
                printf("\n");
                printf("    ** roots of secDer:\n");
                for (auto&& t: rootsOfSecondDerivative) {
                    printf("      ** t = %f ; P''(t) = %13.11f ; P'(t) = %13.11f ; P(t) = %13.11f\n", t,
                           evaluateNthDerivative(n + 2, t), evaluateNthDerivative(n + 1, t),
                           evaluateNthDerivative(n, t));
                }
                printf("\n");
            }
#endif
        }
    }

    return roots[0];
}

template <size_t N>
double PolynomialSolver<N>::evaluateNthDerivative(size_t n, double t) {
    double result = 0.0;
    for (double* it = this->derivatives[n] + N - n; it != this->derivatives[n] - 1; --it) {
        result *= t;
        result += *it;
    }
    return result;
    /**
     * Using the following code results in a 50% increase in computation time for findRoots() of a degree 6 polynomial!
     *   4,274,765 µs vs 6,182,490 µs for 1,000,000 iterations

    return std::accumulate(std::reverse_iterator<const double*>(this->derivatives[n] + N + 1 - n),
                           std::reverse_iterator<const double*>(this->derivatives[n]), 0.0, [t](double r, double s) {
                               r *= t;
                               r += s;
                               return r;
                           });
     */
}

};  // namespace PolynomialSolver
