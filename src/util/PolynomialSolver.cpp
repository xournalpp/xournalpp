#include "PolynomialSolver.h"

#include <cmath>

auto PolynomialSolver::rootsOfQuadratic(double a, double b, double c, double min, double max)
        -> std::vector<double> {
    if (a == 0.0) {
        /**
         * The equation is linear
         */
        if (b != 0.0) {
            double root = -c / (2.0 * b);
            if (root >= min && root <= max) {
                return {root};
            }
            return {};
        }
        return {};
    }

    /**
     * Reduced discriminant of the polynomial equation
     */
    double Delta = b * b - a * c;

    if (Delta > 0) {
        // Sort the two roots from smallest to biggest
        double PMsqrtDelta = (a > 0.0 ? std::sqrt(Delta) : -std::sqrt(Delta));
        double r1 = (-b - PMsqrtDelta) / a;  //
        double r2 = (-b + PMsqrtDelta) / a;  //  r1 < r2
        /**
         * Only keep the root(s) between min and max. Merge-sort test tree
         * Equivalent to return PolynomialSolver::truncate({r1, r2}, min max);
         * 
         * This saves ~49% of execution time for PolynomialSolver::rootsOfQuadratic
         * 
         * ** Benchmark: on 10,000,000 random polynomial equations
         *      With PolynomialSolver::truncate: 7,742,432 µs
         *      With the following code:         3,965,515 µs
         */
        if (r1 >= min) {
            if (r1 <= max) {
                if (r2 <= max) {
                    return {r1, r2};
                } else {
                    return {r1};
                }
            } else {
                return {};
            }
        } else if (r2 >= min && r2 <= max) {
            return {r2};
        }
        return {};
    }

    // if (Delta == 0.0) {
    //     return {-b / a};
    // }
    // Ignore double roots

    return {};
}

auto PolynomialSolver::rootsOfCubic(double a, double b, double c, double d, double min, double max)
        -> std::vector<double> {
    /**
     * Solve the equation a*t^3 + 3*b*t^2 + 3*c*t + d
     * See https://en.wikipedia.org/wiki/Cubic_equation for the methods used here.
     */
    if (a == 0.0) {
        return rootsOfQuadratic(3.0 * b, 1.5 * c, d, min, max);
    }

    /**
     * The equivalent depressed equation u^3 + 3 * p * u + 2 * q = 0 is obtain by the change of variable u = t + b / a
     *
     * Coefficients of the depressed equation
     */
    double minusBOverA = -b / a;
    double bOverASquared = minusBOverA * minusBOverA;
    double cOverA = c / a;
    double p = cOverA - bOverASquared;
    double q = 0.5 * d / a - minusBOverA * (bOverASquared - 1.5 * cOverA);

    /**
     * Discriminant
     */
    double minusDiscriminant = p * p * p + q * q;
    if (minusDiscriminant > 0.0) {
        /**
         * One real solution, two complex ones (which we ignore)
         * Use Cardano's formula
         */
        double sqrtMinusDiscriminant = std::sqrt(minusDiscriminant);
        double root = minusBOverA + std::cbrt(-q + sqrtMinusDiscriminant) + std::cbrt(-q - sqrtMinusDiscriminant);
        if (root >= min && root <= max) {
            return {root};
        }
        return {};
    }
    if (minusDiscriminant == 0.0) {
        /**
         * Fairly improbable case...
         * Three real solutions, at least twice the same
         *
         * p == 0: A triple solution. We don't care for the multiplicity
         * p != 0: A double solution (that we ignore) and a simple one (that we keep)
         */
        double root = (p == 0.0 ? minusBOverA : minusBOverA - 2 * q / p);
        if (root >= min && root <= max) {
            return {root};
        }
        return {};
        /**
         * Nb: q / p = - std::cbrt(q) because minusDiscriminant == 0
         * This trick is used because it reduces the computational cost
         */
    }
    /**
     * minusDiscriminant < 0.0:
     * Three distinct real solutions
     * Use the trigonometric solutions
     */
    double sqrtMinusP = std::sqrt(-p);
    double angle = std::acos(q / (p * sqrtMinusP)) / 3.0;  // in [0, M_PI_3]
    double cosine = sqrtMinusP * std::cos(angle);          // in [0.5 * sqrtMinusP, sqrtMinusP ]
    double sine = SQRT3 * sqrtMinusP * std::sin(angle);    // in [0, 1.5 * sqrtMinusP]
    double r1 = -cosine - sine + minusBOverA;
    double r2 = -cosine + sine + minusBOverA;
    double r3 = 2 * cosine + minusBOverA;
    /**
     *  sqrtMinusP < 2 cosine < 2 sqrtMinusP
     *  -sqrtMinusP < sine - cosine < sqrtMinusP
     *  -2.5 * sqrtMinusP < -sine - cosine < -0.5 * sqrtMinusP
     *  -sine - cosine < sine - cosine
     *
     * Therefore r1 < r2 < r3
     *
     * The code below is equivalent to PolynomialSolver::truncate({r1, r2, r3}, min, max).
     * It follows a standard merge-sort test tree.
     * It avoid functions call and iterator allocations
     * 
     * This save ~30% of execution time of rootsOfCubic
     * ** Benchmark: 10,000,000 random polynomial equations
     *      with PolynomialSolver::truncate: 8,419,412 µs
     *      with the following code:         6,009,103 µs
     */
    if (r1 >= min) {
        if (r1 <= max) {
            if (r2 <= max) {
                if (r3 <= max) {
                    return {r1, r2, r3};
                } else {
                    return {r1, r2};
                }
            } else {
                return {r1};
            }
        } else {
            return {};
        }
    } else if (r2 >= min) {
        if (r2 <= max) {
            if (r3 <= max) {
                return {r2, r3};
            } else {
                return {r2};
            }
        } else {
            return {};
        }
    } else if (r3 >= min && r3 <= max) {
        return {r3};
    }
    return {};
}

// std::vector<double> PolynomialSolver::truncate(const std::vector<double>& input, double min, double max) {
//     auto it1 = std::find_if(input.begin(), input.end(), [min](double t) { return t >= min; });
//     auto it2 = std::find_if(it1, input.end(), [max](double t) { return t > max; });
//     return {it1, it2};
// }

////////////////////////////
#include <chrono>
#include <random>

#include <stdio.h>
using namespace std::chrono;

void PolynomialSolver::test() {
    std::random_device rd;
    std::default_random_engine e(rd());
    std::uniform_real_distribution<double> d(-1.0, 1.0);
    size_t n = 0;

    printf("Loop:        ");
    auto start = high_resolution_clock::now();
    for (int i = 0; i < 10000000; ++i) {
//         printf("\b\b\b\b\b\b\b%7d", i);
        //         printf("\nLoop %d:\n", i);
//         fflush(stdout);
//         PolynomialSolver<6> toto({d(e), d(e), d(e), d(e), d(e), d(e), d(e)});
        //         std::vector<double> tata = toto.findRoots(0.0, 1.0);
        //         std::vector<double> tata = rootsOfCubic(d(e), d(e), d(e),d(e), 0.0, 1.0);
        std::vector<double> tata = rootsOfQuadratic(d(e), d(e),d(e), 0.0, 1.0);
        n += tata.size();
    }
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    printf("\nFound %zu roots in %zu microseconds.\n", n, duration.count());
}
