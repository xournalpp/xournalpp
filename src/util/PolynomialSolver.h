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

#include <array>
#include <numeric>
#include <algorithm>
#include <vector>

#include "Interval.h"

// debug remove!
#include <stdio.h>

namespace PolynomialSolver {
    
constexpr double MAX_ERROR = 1e-6;
constexpr double SQRT3 = 1.7320508075688772935274463415058723669428;

/**
 * @brief Compute the roots (in [min, max]) of the polynomial equation a * t^2 + 2 * b * t + c
 *
 * Warning: double roots are (purposefully and totally) ignored.
 * If the polynomial factors as a * (t - u)^2, the returned vector will be empty
 * [This means we only get roots at which the sign of the polynomial function changes]
 *
 * @param a Quadratic coefficient
 * @param b Half of linear coefficient
 * @param c Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop): ~3,998,731 microseconds for 10,000,000 iterations
 */
std::vector<double> rootsOfQuadratic(double a, double b, double c, double min, double max);

/**
 * @brief Compute the roots (in [min, max]) of the polynomial equation a*t^3 + 3*b*t^2 + 3*c*t + d
 *
 * Warning: double roots are (purposefully and totally) ignored.
 * If the polynomial factors as a * (t - u) * (t - v)^2, the returned vector will only contain u.
 * [This means we only get roots at which the sign of the polynomial function changes]
 *
 * A triple root will be returned (without multiplicities).
 *
 * @param a Cubic coefficient
 * @param b Quadratic coefficient
 * @param c Linear coefficient
 * @param d Constant coefficient
 * @param min Minimal value for the roots
 * @param max Maximal value for the roots
 * @return Vector containing the roots (sorted from smallest to biggest)
 *
 * Benchmark (on a modest laptop): 6,009,103 microseconds for 10,000,000 iterations
 */
std::vector<double> rootsOfCubic(double a, double b, double c, double d, double min, double max);

// std::vector<double> truncate(const std::vector<double>& input, double min, double max);

// Test
void test();


/**************
 * Polynomial *
 **************/
template <size_t N>
class Polynomial {
public:
    Polynomial() = default;
    Polynomial(const std::array<double, N + 1>& coeff): coeff(coeff) {}

    [[maybe_unused]] double evaluate(double t) const;

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
    std::transform(this->coeff.begin(), this->coeff.end() - 1, derivative.coeff.begin(), [&n](double c) { return c * (--n); });
    return derivative;
}

/********************
 * PolynomialSolver *
 ********************/
/**
 * Benchmark (on a modest laptop): 1,000,000 resolutions of degree 6 polynomials in 12,060,228 µs (~12 µs per resolutions)
 */
template <size_t N>
class PolynomialSolver: public Polynomial<N> {
public:
    PolynomialSolver(const Polynomial<N>& poly, const Polynomial<N - 1>& derivative):
    Polynomial<N>(poly), derivative(derivative), secondDerivative(derivative.getDerivative()) {}
    
    PolynomialSolver(const std::array<double, N + 1>& coeff): Polynomial<N>(coeff), derivative(this->getDerivative()), secondDerivative(derivative.getDerivative()) {}
    std::vector<double> findRoots(double min, double max);

    Polynomial<N - 1> derivative;
    Polynomial<N - 2> secondDerivative;
    
    std::vector<double> rootsOfDerivative;
    std::vector<double> rootsOfSecondDerivative;
    std::vector<double> rootsOfThirdDerivative;
};

template <size_t N>
std::vector<double> PolynomialSolver<N>::findRoots(double min, double max) {
    /**
     * First compute the roots in (min, max) of the derivatives
     */
    { // scope for derivativeSolver
        PolynomialSolver<N - 1> derivativeSolver(derivative, secondDerivative);

        this->rootsOfDerivative = derivativeSolver.findRoots(min, max);
        this->rootsOfSecondDerivative.swap(derivativeSolver.rootsOfDerivative);
        this->rootsOfThirdDerivative.swap(derivativeSolver.rootsOfSecondDerivative);
    } // end of scope of derivativeSolver
    
    /**
     * Compute basins of attraction for Halley's method.
     * Those basins are intervals in which P', P'' and P''' do not vanish (assuming P has simple roots)
     * The boolean represents the sign of P at the interval's bound (true = positive, false = negative)
     */
    /**
     * First find intervals that must contain exactly one root and on which P' does not vanish
     */
    std::vector<Interval<std::pair<double, bool>>> basins;
    if (this->rootsOfDerivative.empty()) {
        if (bool signMin = this->evaluate(min) > 0.0, signMax = this->evaluate(max) > 0.0; signMin != signMax) {
            basins = {{{min, signMin}, {max, signMax}}};
        } else {
            // The function is monotonous and has same sign on endpoints: it does not have roots between min and max
            return {};
        }
    } else {
        basins.reserve(this->rootsOfDerivative.size() + 1);
        double firstRoot = this->rootsOfDerivative.front();
        double lastSign = this->evaluate(firstRoot) > 0.0;
        if (bool signMin = this->evaluate(min) > 0.0; signMin != lastSign) {
            basins.emplace_back(std::make_pair(min, signMin), std::make_pair(firstRoot, lastSign));
        }
        for (auto it1 = this->rootsOfDerivative.begin(), it2 = it1 + 1; it2 != this->rootsOfDerivative.end() ; it1 = it2++) {
            bool sign = this->evaluate(*it2) > 0.0;
            if (sign != lastSign) {
                /**
                 * P(*it1) and P(*it2) have different signs (and P'(t) != 0 for t in (*it1, *it2))
                 * Keep this interval
                 */
                basins.emplace_back(std::make_pair(*it1, lastSign), std::make_pair(*it2, sign));
                lastSign = sign;
            }
        }
        if (bool signMax = this->evaluate(max) > 0.0; signMax != lastSign) {
            basins.emplace_back(std::make_pair(this->rootsOfDerivative.back(), lastSign), std::make_pair(max, signMax));
        }
    }
    /**
     * Shrink those interval so that P'' does not vanish
     */
    if (!this->rootsOfSecondDerivative.empty()) {
        auto it = this->rootsOfSecondDerivative.begin();
        auto end = this->rootsOfSecondDerivative.end();
        for (auto&& basin: basins) {
            while (it != end && basin.max.first > *it) {
                if (basin.min.first < *it) {
                    if (bool sign = this->evaluate(*it) > 0.0; sign != basin.min.second) {
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
     * Shrink those interval so that P''' does not vanish
     */
    if (!this->rootsOfThirdDerivative.empty()) {
        auto it = this->rootsOfThirdDerivative.begin();
        auto end = this->rootsOfThirdDerivative.end();
        for (auto&& basin: basins) {
            while (it != end && basin.max.first > *it) {
                if (basin.min.first < *it) {
                    if (bool sign = this->evaluate(*it) > 0.0; sign != basin.min.second) {
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
     * Now compute the root within each basin
     */
    std::vector<double> result;
    result.reserve(basins.size());
    for (auto&& basin: basins) {
        double t = 0.5 * (basin.min.first + basin.max.first);
        double value = this->evaluate(t);
        size_t n = 0;
        
        while (std::abs(value) > MAX_ERROR && n < 10) {
            n++;
            /* Halley iteration */
            double derValue = derivative.evaluate(t);
            double secValue = secondDerivative.evaluate(t);
            t -= value * derValue / (derValue * derValue - 0.5 * value * secValue);
            /********************/
            value = this->evaluate(t);
        }
        result.push_back(t);

#define POLY_DEBUG
#ifdef POLY_DEBUG
        if (n == 10) {
            printf("\ndegree %zu. Iterations: %zu. t = %f. value = %f\n", N, n, t, value);
            printf("*** Polynomial: ");
            for (size_t i = 0; i <= N; i++) {
                printf("%f ; ", this->coeff[i]);
            }
            printf("\n");
            printf("  ** Interval: [%f ; %f]\n", min, max);
            
            printf("  ** Derivative: ");
            for (size_t i = 0; i < N; i++) {
                printf("%f ; ", derivative.coeff[i]);
            }
            printf("\n");
            printf("    ** roots of der:\n");
            for (auto&& t: rootsOfDerivative) {
                printf("      ** t = %f ; P'(t) = %f ; P(t) = %f\n", t, derivative.evaluate(t), this->evaluate(t));
            }
            printf("\n");
            printf("  ** Second Derivative: ");
            for (size_t i = 0; i < N-1; i++) {
                printf("%f ; ", secondDerivative.coeff[i]);
            }
            printf("\n");
            printf("    ** roots of secDer:\n");
            for (auto&& t: rootsOfSecondDerivative) {
                printf("      ** t = %f ; P''(t) = %f ; P(t) = %f\n", t, secondDerivative.evaluate(t), this->evaluate(t));
            }
            printf("\n");
            printf("\n");
        }
#endif
    }
    return result;
}

/**
 * For cubic polynomial equations, using the exact method here is about 25% faster. Overload the solver.
 * 
 * Nb: This class is used in the recursive resolution of higher degree polynomials.
 * For solving a given cubic equation, use PolynomialSolver::rootsOfCubic instead which is another 17% faster.
 */
template <>
class PolynomialSolver<3>: public Polynomial<3> {
public:
    PolynomialSolver(Polynomial<3> poly, Polynomial<2> derivative):
    Polynomial<3>(poly) {}

    PolynomialSolver(const std::array<double, 4>& coeff): Polynomial<3>(coeff) {}

    std::vector<double> findRoots(double min, double max) {
        this->rootsOfDerivative = rootsOfQuadratic(coeff[0] * 3.0, coeff[1], coeff[2], min, max);
        this->rootsOfSecondDerivative.clear();
        if (coeff[0] != 0.0) {
            double root = - coeff[1] / (3.0 * coeff[0]);
            if (root > min && root < max) {
                this->rootsOfSecondDerivative = {root};
            }
        }
        return rootsOfCubic(coeff[0], coeff[1] / 3.0, coeff[2] / 3.0, coeff[3], min, max);
    }

    std::vector<double> rootsOfDerivative;
    std::vector<double> rootsOfSecondDerivative;
};

template <>
class PolynomialSolver<2>: public Polynomial<2> {
public:
    PolynomialSolver(const Polynomial<2>& poly, const Polynomial<1>& derivative):
    Polynomial<2>(poly) {}

    PolynomialSolver(const std::array<double, 3>& li): Polynomial<2>(li) {}

    std::vector<double> findRoots(double min, double max) {
        return rootsOfQuadratic(coeff[0], 0.5 * coeff[1], coeff[2], min, max);
    }
};

template <>
class PolynomialSolver<1>: public Polynomial<1> {
public:
    PolynomialSolver(const Polynomial<1>& poly, const Polynomial<0>& derivative):
    Polynomial<1>(poly) {}
    
    PolynomialSolver(const std::array<double, 2>& coeff): Polynomial<1>(coeff) {}
    
    std::vector<double> findRoots(double min, double max) {
        if (coeff[0] == 0.0) {
            return {};
        }
        double root = -coeff[1] / coeff[0];
        if (root >= min && root <= max) {
            return {root};
        }
        return {};
    }
};

};  // namespace PolynomialSolver
