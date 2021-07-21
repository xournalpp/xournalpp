/*
 * Xournal++
 *
 * A solver of polynomial equations
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

/**
 * NOTICE: Benchmark is available as a disabled (g)test unit (see test/unit_tests/util/PolynomialSolverBenchmark.cpp)
 *      To run this benchmark:
 *          $ test/test-units --gtest_filter=*benchmark*  --gtest_also_run_disabled_tests
 */

#pragma once

#include <algorithm>         // for transform
#include <array>             // for array
#include <cmath>             // for abs
#include <cstddef>           // for size_t, ptrdiff_t
#include <initializer_list>  // for initializer_list
#include <iterator>          // for next, prev
#include <numeric>           // for accumulate
#include <utility>           // for pair

#include "Interval.h"    // for Interval
#include "TinyVector.h"  // for TinyVector

// #define POLY_DEBUG
// #define POLY_DEBUG_CONVERGENCE

#if defined(POLY_DEBUG) || defined(POLY_DEBUG_CONVERGENCE)
#include <cstdio>
#endif

namespace PolynomialSolver {

constexpr double FUZZY_ZERO = 1e-20;  // A too big value will result in numerical instability (e.g. 1e-10 is too big...)
constexpr double BIG_FUZZY_ZERO = 1e-10;  // And for some things, FUZZY_ZERO is too small...
constexpr double MAX_ERROR = 1e-6;
constexpr double SQRT3 = 1.7320508075688772935274463415058723669428;

/**
 * @brief Compute the roots (in [min, max]) of the polynomial equation a * t^2 + 2 * b * t + c
 *
 * Warning: double roots are (purposefully) ignored.
 * If the polynomial factors as a * (t - u)^2, the returned vector will be empty
 * [This means we only get roots at which the sign of the polynomial function changes]
 *
 * Warning: Very small values for the leading coefficient 'a' will result in very high numerical errors
 *
 * @param a Quadratic coefficient
 * @param b Half of linear coefficient
 * @param c Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop, with -O2): 697,257 microseconds for 10,000,000 iterations
 */
TinyVector<double, 2> rootsOfQuadratic(double a, double b, double c, double min, double max);

/**
 * @brief Compute the roots (in [min, max]) of the polynomial equation a*t^3 + 3*b*t^2 + 3*c*t + d
 *
 * Warning: double roots are (purposefully) ignored.
 * If the polynomial factors as a * (t - u) * (t - v)^2, the returned vector will only contain u.
 * [This means we only get roots at which the sign of the polynomial function changes]
 *
 * A triple root will be returned (without multiplicities).
 *
 * Warning: Very small values for the leading coefficient 'a' will result in very high numerical errors
 *
 * @param a Cubic coefficient
 * @param b Third of quadratic coefficient
 * @param c Third of linear coefficient
 * @param d Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop, with -O2): 1,538,142 microseconds for 10,000,000 iterations
 */
TinyVector<double, 3> rootsOfCubic(double a, double b, double c, double d, double min, double max);

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
 * Benchmark (on a modest laptop, with -O2): 1,985,920 microseconds for 10,000,000 iterations
 */
TinyVector<double, 4> rootsOfQuartic(double a, double b, double c, double d, double e, double min, double max);

/**
 * @brief Returns a sorted container containing the elements of the provided list that are between min and max.
 *          Assumes the input list is sorted.
 * @param min Lower bound
 * @param max Upper bound
 * @return A sorted container containing the elements between min and max.
 */
template <unsigned int N>
TinyVector<double, N> clampToTinyVector(std::initializer_list<double> input, double min, double max);

/**************
 * Polynomial *
 **************/
template <size_t N>
class Polynomial {
public:
    Polynomial() = default;

    double evaluate(double t) const;

    std::pair<double, double> getMinimum(double min, double max);

    Polynomial<N - 1> getDerivative() const;

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
 * @brief Solver class for polynomials of degree > 4
 *
 * Benchmark (on a modest laptop, with -O2):
 *  * 10,000,000 resolutions of degree 6 polynomials in 4,402,112 µs
 *  * 10,000,000 resolutions of degree 5 polynomials in 6,153,181 µs
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
    PolynomialSolver(const Polynomial<N>& poly);

    using RootContainer = TinyVector<double, N>;
    /**
     * @brief Compute the roots of the polynomial that lie between min and max.
     * @param min Lower bound for roots of interest
     * @param max Upper bound for roots of interest
     * @return A vector containing the roots.
     */
    RootContainer findRoots(double min, double max) const;

private:
    /**
     * @brief Type for basins of attraction for Halley's method.
     * Those basins are intervals in which Q', Q'' and Q''' do not vanish (assuming Q has simple roots)
     */
    struct BasinBound {
        double t;
        // Sign of the polynomial at t. true = positive. false = negative
        bool sign;
    };
    using Basin = Interval<BasinBound>;
    using BasinContainer = TinyVector<Basin, N>;

    /**
     * @brief Compute P^{(n)}(t) (the value of the n-th derivative at t)
     * @param n Rank of the derivative
     * @param t Parameter at which we evaluate
     * @return The value P^{(n)}(t)
     */
    double evaluateNthDerivative(size_t n, double t) const;

    /**
     * @brief Get 3 containers with the roots of the iterated derivatives P^{(N-2)}, P^{(N-3)} and P^{(N-4)}.
     *     Ignore roots outside of the interval (min,max]
     */
    std::array<RootContainer, 3> getInitialRootContainer(double min, double max) const;

    /**
     * @brief Get basins (= intervals) where the n-th derivative P^{(n)} is monotonous
     *      Only returns basins within the interval (min, max]
     * @param rootsOfDerivative Container with the roots of P^{(n+1)}
     */
    BasinContainer getMonotonousBasinsForNthDerivative(const size_t n, const double min, const double max,
                                                       const RootContainer& rootsOfDerivative) const;

    /**
     * @brief Shrink the provided basins so that they no longer contain values from rootsOfHigherDerivative
     */
    void shrinkBasinsSoThatTheHigherDerivativeDoesNotVanish(size_t n, BasinContainer& basins,
                                                            const RootContainer& rootsOfHigherDerivative) const;

    /**
     * @brief Compute the root of P^{(n)} contained in the given basin and push it to the RootContainer
     *      Assumes the basin is such that
     *          * P^{(n)} changes sign (exactly once) in the basin
     *          * P^{(n+1)}, P^{(n+2)} and P^{(n+3)} do not vanish in the basin
     *      Those conditions ensure convergence of the Halley iterations
     */
    void pushRootFromBasin(size_t n, const Basin& basin, RootContainer& roots) const;

    /**
     * @brief Array containing the coefficients of the iterated derivatives P = P^{(0)}, P^{(1)}, ..., P^{(N-2)} of the
     * target polynomial equation P.
     */
    static constexpr size_t NB_COEFF = (N + 1) * (N + 2) / 2 - 3;
    std::array<double, NB_COEFF> derivativesCoefficients;

    using CoeffIterator = typename std::array<double, NB_COEFF>::iterator;
    using ConstCoeffIterator = typename std::array<double, NB_COEFF>::const_iterator;

    /**
     * @brief Array containing iterators to the first (=highest degree) coefficient of each derivative
     * NB: derivatives.back() is the end of the last derivatives' coeff. It is not dereferencable.
     */
    std::array<CoeffIterator, N> derivatives;

    inline CoeffIterator getNthDerivativeBegin(size_t n) { return derivatives[n]; }
    inline CoeffIterator getNthDerivativeEnd(size_t n) { return derivatives[n + 1]; }
    inline ConstCoeffIterator getNthDerivativeBegin(size_t n) const { return derivatives[n]; }
    inline ConstCoeffIterator getNthDerivativeEnd(size_t n) const { return derivatives[n + 1]; }

#ifdef POLY_DEBUG
    template <typename container>
    void debugPrintNthDerivativeAndRoots(size_t n, const container& roots) const {
        printf("* P^{(%zu)}(t) =", n);
        size_t deg = N - n;
        for (double* it = derivatives[n] + N - n; it >= derivatives[n]; --it, --deg) {
            printf(" %f*t^%zu +", *it, deg);
        }
        printf("\b\b\b\b    \n");
        printf("    ROOTS:\n");
        for (auto t: roots) {
            printf("     t = %f   ;   P^{(%zu)}(t) = %13.11f\n", t, n, evaluateNthDerivative(n, t));
        }
        printf("\n");
    }
#endif
#ifdef POLY_DEBUG_CONVERGENCE
    void debugPrintConvergenceInfo(const Basin& basin, double allegedRoot, size_t n,
                                   const RootContainer& rootsOfDerivative,
                                   const RootContainer& rootsOfSecondDerivative) const {
        const double value = evaluateNthDerivative(n, allegedRoot);
        if (basin.min.t > allegedRoot || basin.max.t < allegedRoot || std::abs(value) > MAX_ERROR) {
            printf("\n");
            printf("Degree %zu", N - n);
            printf(" -- Basin: [%f ; %f)\n", basin.min.t, basin.max.t);
            printf(" -- val: [%13.11f ; %13.11f)\n", evaluateNthDerivative(n, basin.min.t),
                   evaluateNthDerivative(n, basin.max.t));
            printf("  P(t) =");
            size_t deg = N - n;
            for (const double* it = derivatives[n] + N - n; it >= derivatives[n]; --it, --deg) {
                printf(" %f*t^%zu", *it, deg);
                if (deg != 0u) {
                    printf(" +");
                }
            }
            printf("\n");
            printf("    ** root found t = %f ; P(t) = %13.11f\n", allegedRoot, value);
            printf("    ** roots of P':\n");
            for (auto&& t: rootsOfDerivative) {
                printf("      ** t = %f ; P''(t) = %13.11f ; P'(t) = %13.11f ; P(t) = %13.11f\n", t,
                       evaluateNthDerivative(n + 2, t), evaluateNthDerivative(n + 1, t), evaluateNthDerivative(n, t));
            }
            printf("\n");
            printf("    ** roots of P'':\n");
            for (auto&& t: rootsOfSecondDerivative) {
                printf("      ** t = %f ; P''(t) = %13.11f ; P'(t) = %13.11f ; P(t) = %13.11f\n", t,
                       evaluateNthDerivative(n + 2, t), evaluateNthDerivative(n + 1, t), evaluateNthDerivative(n, t));
            }
            printf("\n");
        }
    }
#endif
};

template <size_t N>
PolynomialSolver<N>::PolynomialSolver(const Polynomial<N>& poly) {
    std::copy(poly.coeff.begin(), poly.coeff.end(), derivativesCoefficients.begin());

    CoeffIterator coeffIt = derivativesCoefficients.begin();
    ptrdiff_t n = N + 1;
    for (auto& it: derivatives) {
        it = coeffIt;
        coeffIt += n--;
    }

    double multiplier = N + 2;
    for (auto it1 = derivatives.begin(), it2 = std::next(it1), end = std::prev(derivatives.end()); it2 != end;
         it1 = it2++) {
        double m = --multiplier;
        std::transform(*it1, std::prev(*it2), *it2, [&m](double c) { return c * (--m); });
    }
}

template <size_t N>
double PolynomialSolver<N>::evaluateNthDerivative(size_t n, double t) const {
    return std::accumulate(getNthDerivativeBegin(n), getNthDerivativeEnd(n), 0.0,
                           [t](double r, double s) { return t * r + s; });
}

template <size_t N>
auto PolynomialSolver<N>::getInitialRootContainer(double min, double max) const -> std::array<RootContainer, 3> {
    // Compute the roots in (min, max) of the N-2nd, N-3rd and N-4th derivatives
    auto deg2Der = getNthDerivativeBegin(N - 2);
    auto deg3Der = getNthDerivativeBegin(N - 3);
    auto deg4Der = getNthDerivativeBegin(N - 4);

    std::array<RootContainer, 3> roots;
    roots[(N + 1) % 3] = rootsOfQuadratic(deg2Der[0], 0.5 * deg2Der[1], deg2Der[2], min, max);
    roots[N % 3] = rootsOfCubic(deg3Der[0], deg3Der[1] / 3.0, deg3Der[2] / 3.0, deg3Der[3], min, max);
    roots[(N + 2) % 3] =
            rootsOfQuartic(deg4Der[0], 0.25 * deg4Der[1], deg4Der[2] / 6.0, 0.25 * deg4Der[3], deg4Der[4], min, max);

#ifdef POLY_DEBUG
    debugPrintNthDerivativeAndRoots(N - 2, roots[(N + 1) % 3]);
    debugPrintNthDerivativeAndRoots(N - 3, roots[N % 3]);
    debugPrintNthDerivativeAndRoots(N - 4, roots[(N + 2) % 3]);
#endif

    return roots;
};

template <size_t N>
auto PolynomialSolver<N>::getMonotonousBasinsForNthDerivative(const size_t n, const double min, const double max,
                                                              const RootContainer& rootsOfDerivative) const
        -> BasinContainer {
    /**
     * Get attraction basins that contain a root and where the function is monotonous
     */
    if (rootsOfDerivative.empty()) {
        // The polynomial is monotonous on [min,max]
        if (bool signMin = evaluateNthDerivative(n, min) > 0.0, signMax = evaluateNthDerivative(n, max) > 0.0;
            signMin != signMax) {
            return {Basin({min, signMin}, {max, signMax})};
        }
        return {};
    }

    BasinContainer basins;
    double firstRoot = rootsOfDerivative.front();
    bool lastSign = evaluateNthDerivative(n, firstRoot) > 0.0;
    if (bool signMin = evaluateNthDerivative(n, min) > 0.0; signMin != lastSign) {
        basins.emplace_back(Basin({min, signMin}, {firstRoot, lastSign}));
    }
    for (auto it1 = rootsOfDerivative.begin(), it2 = std::next(it1); it2 != rootsOfDerivative.end(); it1 = it2++) {
        bool sign = evaluateNthDerivative(n, *it2) > 0.0;
        if (sign != lastSign) {
            /**
             * Q(*it1) and Q(*it2) have different signs (and Q'(t) != 0 for t in (*it1, *it2))
             * Keep this interval: it contains exactly one root
             */
            basins.emplace_back(Basin({*it1, lastSign}, {*it2, sign}));
            lastSign = sign;
        }
    }
    if (bool signMax = evaluateNthDerivative(n, max) > 0.0; signMax != lastSign) {
        basins.emplace_back(Basin({rootsOfDerivative.back(), lastSign}, {max, signMax}));
    }
    return basins;
};

template <size_t N>
void PolynomialSolver<N>::shrinkBasinsSoThatTheHigherDerivativeDoesNotVanish(
        size_t n, BasinContainer& basins, const RootContainer& rootsOfHigherDerivative) const {
    /**
     * Shrink the basins so that Q'' or Q''' does not vanish
     */
    if (!rootsOfHigherDerivative.empty()) {
        auto it = rootsOfHigherDerivative.begin();
        auto end = rootsOfHigherDerivative.end();
        for (auto&& basin: basins) {
            while (it != end && basin.max.t > *it) {
                if (basin.min.t < *it) {
                    if (bool sign = evaluateNthDerivative(n, *it) > 0.0; sign != basin.min.sign) {
                        basin.max = {*it, sign};
                    } else {
                        basin.min = {*it, sign};
                    }
                }
                ++it;
            }
            if (it == end) {
                return;
            }
        }
    }
};

template <size_t N>
void PolynomialSolver<N>::pushRootFromBasin(size_t n, const Basin& basin, RootContainer& roots) const {
    double t = 0.5 * (basin.min.t + basin.max.t);
    double value = evaluateNthDerivative(n, t);
    size_t count = 0;

    while (std::abs(value) > MAX_ERROR && count < 10) {
        count++;
        /* Halley iteration */
        double derValue = evaluateNthDerivative(n + 1, t);
        double secValue = evaluateNthDerivative(n + 2, t);
        t -= value * derValue / (derValue * derValue - 0.5 * value * secValue);
        value = evaluateNthDerivative(n, t);
    }
    roots.push_back(t);
};

template <size_t N>
auto PolynomialSolver<N>::findRoots(double min, double max) const -> RootContainer {
    /**
     * For the duration of the following loop, this will contain the roots (between min and max) of the iterated
     * derivatives P^{(n)}, P^{(n+1)} and P^{(n+2)} (up to a cyclic permutation).
     */
    auto roots = getInitialRootContainer(min, max);

    for (size_t n = N - 4; n != 0;) {
        --n;  // n == 0 in the last iteration

        /**
         * Find the roots of Q = P^{(n)} the n-th derivative of our target polynomial equation.
         * The roots of Q', Q'' and Q''' are already computed and stored in `roots`
         */

        //  Aliases for the root vectors
        const auto& rootsOfQPrime = roots[(n + 1) % 3];   // Q'
        const auto& rootsOfQSecond = roots[(n + 2) % 3];  // Q''
        const auto& rootsOfQThird = roots[n % 3];         // Q'''

        auto basins = getMonotonousBasinsForNthDerivative(n, min, max, rootsOfQPrime);
        shrinkBasinsSoThatTheHigherDerivativeDoesNotVanish(n, basins, rootsOfQSecond);
        shrinkBasinsSoThatTheHigherDerivativeDoesNotVanish(n, basins, rootsOfQThird);

        auto& rootsOfQ = roots[n % 3];
        rootsOfQ.clear();

        for (auto&& basin: basins) {
            pushRootFromBasin(n, basin, rootsOfQ);
#ifdef POLY_DEBUG_CONVERGENCE
            debugPrintConvergenceInfo(basin, rootsOfQ.back(), n, rootsOfDerivative, rootsOfSecondDerivative);
#endif
        }
#ifdef POLY_DEBUG
        debugPrintNthDerivativeAndRoots(n, rootsOfQ);
#endif
    }

    return roots[0];
}
};  // namespace PolynomialSolver
