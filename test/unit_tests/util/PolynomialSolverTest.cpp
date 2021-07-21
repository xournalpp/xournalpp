#include <array>    // for array
#include <cstdlib>  // for abs, size_t
#include <vector>   // for vector

#include <gtest/gtest.h>  // for Test, ASSERT_EQ, ASSERT_TRUE

#include "util/PolynomialSolver.h"  // for BIG_FUZZY_ZERO, rootsOfCubic
#include "util/TinyVector.h"        // for TinyVector

template <size_t degree>
struct testCase {
    std::array<double, degree + 1> coeffs;
    TinyVector<double, degree> solutions;
};

TEST(UtilPolynomialSolver, testLowDegrees) {

    {  // Quadratic polynomials
        std::vector<testCase<2>> quadraticPolynomials(8);

        //  x^2 + x + 1 no real solutions
        quadraticPolynomials[0].coeffs = {1.0, 1.0, 1.0};
        quadraticPolynomials[0].solutions = {};

        //  x^2 + 2 * x - 3 = (x + 3)*(x - 1)
        quadraticPolynomials[1].coeffs = {1.0, 2.0, -3.0};
        quadraticPolynomials[1].solutions = {-3.0, 1.0};

        //  x^2 - 2 * x - 3 = (x - 3)*(x + 1)
        quadraticPolynomials[2].coeffs = {1.0, -2.0, -3.0};
        quadraticPolynomials[2].solutions = {-1.0, 3.0};

        //  x^2 - 5 * x - 6 = (x - 6)*(x + 1)
        quadraticPolynomials[3].coeffs = {1.0, -5.0, -6.0};
        quadraticPolynomials[3].solutions = {-1.0};  // Only within (-5, 5)

        //  3 * x^2 + 9 * x - 54 = 3*(x - 3)*(x + 6)
        quadraticPolynomials[4].coeffs = {3.0, 9.0, -54.0};
        quadraticPolynomials[4].solutions = {3.0};  // Only within (-5, 5)

        //  x^2 + x - 42 = (x - 6)*(x - 7)
        quadraticPolynomials[5].coeffs = {1.0, -13.0, 42.0};
        quadraticPolynomials[5].solutions = {};  // Only within (-5, 5)

        //  x^2 - 2 * x + 1 = (x - 1)*(x - 1)
        quadraticPolynomials[6].coeffs = {1.0, -2.0, 1.0};
        quadraticPolynomials[6].solutions = {};  // Ignore double roots

        // 2*x + 1  linear case
        quadraticPolynomials[7].coeffs = {PolynomialSolver::BIG_FUZZY_ZERO / 2.0, 2.0, 1.0};
        quadraticPolynomials[7].solutions = {-0.5};

        for (auto& tc: quadraticPolynomials) {
            auto sol = PolynomialSolver::rootsOfQuadratic(tc.coeffs[0], tc.coeffs[1] / 2.0, tc.coeffs[2], -5.0, 5.0);
            ASSERT_EQ(sol.size(), tc.solutions.size());

            auto it = tc.solutions.begin();
            for (auto s: sol) {
                ASSERT_TRUE(std::abs(s - *it++) < PolynomialSolver::BIG_FUZZY_ZERO);
            }
        }
    }

    {  // Cubic polynomials
        std::vector<testCase<3>> cubicPolynomials(7);

        // 3*(x^3 - 1) one real sol. 2 complex ones
        cubicPolynomials[0].coeffs = {3.0, 0.0, 0.0, -3.0};
        cubicPolynomials[0].solutions = {1.0};

        // 4*(x-1)(x+1)^2 double root
        cubicPolynomials[1].coeffs = {4.0, 4.0, -4.0, -4.0};
        cubicPolynomials[1].solutions = {1.0};  // Ignore double roots

        // (x-2)^3 triple root
        cubicPolynomials[2].coeffs = {1.0, -6.0, 12.0, -8.0};
        cubicPolynomials[2].solutions = {2.0};  // No multiplicities

        // (x-1)(x-2)(x-6)
        cubicPolynomials[3].coeffs = {1.0, -9.0, 20.0, -12.0};
        cubicPolynomials[3].solutions = {1.0, 2.0};  // Only within (-5, 5)

        // (x-10)(x+7)(x-6)
        cubicPolynomials[4].coeffs = {1.0, -9.0, -52.0, 420.0};
        cubicPolynomials[4].solutions = {};  // Only within (-5, 5)

        // (x-1)(x+2) Quadratic case
        cubicPolynomials[5].coeffs = {PolynomialSolver::BIG_FUZZY_ZERO / 2.0, 1.0, 1.0, -2.0};
        cubicPolynomials[5].solutions = {-2.0, 1.0};

        // 4x - 1 Linear case
        cubicPolynomials[6].coeffs = {PolynomialSolver::BIG_FUZZY_ZERO / 2.0, PolynomialSolver::BIG_FUZZY_ZERO / 2.0,
                                      4.0, -1.0};
        cubicPolynomials[6].solutions = {0.25};

        for (auto& tc: cubicPolynomials) {
            auto sol = PolynomialSolver::rootsOfCubic(tc.coeffs[0], tc.coeffs[1] / 3.0, tc.coeffs[2] / 3.0,
                                                      tc.coeffs[3], -5.0, 5.0);
            ASSERT_EQ(sol.size(), tc.solutions.size());

            auto it = tc.solutions.begin();
            for (auto s: sol) {
                ASSERT_TRUE(std::abs(s - *it++) < PolynomialSolver::BIG_FUZZY_ZERO);
            }
        }
    }

    {  // Quartic polynomials
        std::vector<testCase<4>> quarticPolynomials(5);

        // (x-10)(x+7)(x-6)(x-1)
        quarticPolynomials[0].coeffs = {1.0, -10.0, -43.0, 472.0, -420.0};
        quarticPolynomials[0].solutions = {1.0};  // Only within (-5, 5)

        // 2(x-1)^3(x+2) triple root
        quarticPolynomials[1].coeffs = {2.0, -2.0, -6.0, 10.0, -4.0};
        quarticPolynomials[1].solutions = {-2.0, 1.0};  // No multiplicities

        // (x-1)^2(x+2)^2 two pairs of double roots
        quarticPolynomials[2].coeffs = {1.0, 2.0, -3.0, -4.0, 4.0};
        quarticPolynomials[2].solutions = {};  // Ignore double roots

        // (x-1)^2(x+1)(x+2)
        quarticPolynomials[3].coeffs = {1.0, 1.0, -3.0, -1.0, 2.0};
        quarticPolynomials[3].solutions = {-2.0, -1.0};  // Ignore double roots

        // (x-1)(x+1)(x+2) Cubic case
        quarticPolynomials[4].coeffs = {PolynomialSolver::BIG_FUZZY_ZERO / 2.0, 1.0, 2.0, -1.0, -2.0};
        quarticPolynomials[4].solutions = {-2.0, -1.0, 1.0};

        for (auto& tc: quarticPolynomials) {
            auto sol = PolynomialSolver::rootsOfQuartic(tc.coeffs[0], tc.coeffs[1] / 4.0, tc.coeffs[2] / 6.0,
                                                        tc.coeffs[3] / 4.0, tc.coeffs[4], -5.0, 5.0);
            ASSERT_EQ(sol.size(), tc.solutions.size());

            auto it = tc.solutions.begin();
            for (auto s: sol) {
                ASSERT_TRUE(std::abs(s - *it++) < PolynomialSolver::BIG_FUZZY_ZERO);
            }
        }
    }
}

TEST(UtilPolynomialSolver, testHighDegrees) {
    std::vector<testCase<10>> polynomials(2);

    // (x-1)(x-2)(x-3)(x-4)(x-5.5)(x-6)(x+1)(x+1.5)(x+2)(x+6)
    polynomials[0].coeffs = {1.0, -11.0, -9.25, 460.75, -1216.75, -2423.75, 9757.0, 3378.0, -22788.0, -1404.0, 14256.0};
    polynomials[0].solutions = {-2.0, -1.5, -1.0, 1.0, 2.0, 3.0, 4.0};

    // (x^3-1)(x-2)(x+1)(x+6)(x+2)(x^2+1)x  With complex roots
    polynomials[1].coeffs = {1.0, 7.0, 3.0, -22.0, -29.0, -31.0, -3.0, 22.0, 28.0, 24.0, 0.0};
    polynomials[1].solutions = {-2.0, -1.0, 0.0, 1.0, 2.0};

    for (auto& tc: polynomials) {
        PolynomialSolver::Polynomial<10> poly;
        std::copy(tc.coeffs.begin(), tc.coeffs.end(), poly.coeff.begin());
        PolynomialSolver::PolynomialSolver<10> solver(poly);
        auto sol = solver.findRoots(-5.0, 5.0);
        ASSERT_EQ(sol.size(), tc.solutions.size());

        auto it = tc.solutions.begin();
        for (auto s: sol) {
            ASSERT_TRUE(std::abs(s - *it++) < PolynomialSolver::BIG_FUZZY_ZERO);
        }
    }
}
