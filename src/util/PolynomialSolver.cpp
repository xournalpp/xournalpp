#include "util/PolynomialSolver.h"

#include <algorithm>  // for copy_if, copy

#include "util/TinyVector.h"  // for TinyVector

auto PolynomialSolver::rootsOfQuadratic(double a, double b, double c, double min, double max) -> TinyVector<double, 2> {
    if (std::abs(a) < BIG_FUZZY_ZERO) {
        /**
         * The equation is linear
         */
        if (std::abs(b) > FUZZY_ZERO) {
            double root = -c / (2.0 * b);
            if (root >= min && root < max) {
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

    if (Delta > FUZZY_ZERO) {
        // Sort the two roots from smallest to biggest
        double PMsqrtDelta = (a > 0.0 ? std::sqrt(Delta) : -std::sqrt(Delta));
        return clampToTinyVector<2>({(-b - PMsqrtDelta) / a, (-b + PMsqrtDelta) / a}, min, max);
    }

    return {};
}

auto PolynomialSolver::rootsOfCubic(double a, double b, double c, double d, double min, double max)
        -> TinyVector<double, 3> {
    /**
     * Solve the equation a*t^3 + 3*b*t^2 + 3*c*t + d
     * See https://en.wikipedia.org/wiki/Cubic_equation for the methods used here.
     */
    if (std::abs(a) < BIG_FUZZY_ZERO) {
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
    if (minusDiscriminant > FUZZY_ZERO) {
        /**
         * One real solution, two complex ones (which we ignore)
         * Use Cardano's formula
         */
        double sqrtMinusDiscriminant = std::sqrt(minusDiscriminant);
        double root = minusBOverA + std::cbrt(-q + sqrtMinusDiscriminant) + std::cbrt(-q - sqrtMinusDiscriminant);
        if (root >= min && root < max) {
            return {root};
        }
        return {};
    }
    if (minusDiscriminant > -FUZZY_ZERO) {  // fuzzy vanish
        /**
         * Fairly improbable case...
         * Three real solutions, at least twice the same
         *
         * p == 0: A triple solution. We don't care for the multiplicity
         * p != 0: A double solution (that we ignore) and a simple one (that we keep)
         */
        double root = (std::abs(p) < FUZZY_ZERO ? minusBOverA : minusBOverA + 2 * q / p);
        if (root >= min && root < max) {
            return {root};
        }
        return {};
        /**
         * Nb: q / p = - std::cbrt(q) because minusDiscriminant == 0
         * This trick is used to reduce the computational cost
         */
    }
    /**
     * minusDiscriminant <= -FUZZY_ZERO:
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
     */
    return clampToTinyVector<3>({r1, r2, r3}, min, max);
}

auto PolynomialSolver::rootsOfQuartic(double a, double b, double c, double d, double e, double min, double max)
        -> TinyVector<double, 4> {
    if (std::abs(a) < BIG_FUZZY_ZERO) {
        return rootsOfCubic(4.0 * b, 2.0 * c, 4.0 / 3.0 * d, e, min, max);
    }

    /**
     * Use Ferrari's method
     *
     * First normalize the coefficients.
     */
    double bOverA = b / a;
    double cOverA = c / a;
    double dOverA = d / a;
    double eOverA = e / a;

    double quarticShift = -bOverA;

    double cOverASquared = cOverA * cOverA;
    double bOverASquared = bOverA * bOverA;
    double bOverATimesdOverA = bOverA * dOverA;

    // Coefficients of the depressed quartic t^4 + 6*quarticP*t^2 + 4*quarticQ*t + quarticR
    double quarticP = cOverA - bOverASquared;
    double quarticQ = dOverA - 3.0 * bOverA * cOverA + 2.0 * bOverASquared * bOverA;
    // Unused in the general case
    // double quarticR = eOverA - 4.0 * bOverATimesdOverA + 6.0 * bOverASquared * cOverA - 3.0 * bOverASquared *
    // bOverASquared;


    bool solveAsBiquadratic = std::abs(quarticQ) < FUZZY_ZERO;

    double biggestRoot = 0.0;
    if (!solveAsBiquadratic) {
        /**
         * Find the biggest real root of the (depressed) resolvent cubic:
         * t^3 + 3*cubicP*t + 2*cubicQ = 0
         */
        double cubicP = (4.0 * bOverATimesdOverA - eOverA) / 3.0 - cOverASquared;
        double cubicQ = quarticP * eOverA - dOverA * dOverA + 2.0 * bOverATimesdOverA * cOverA - cOverA * cOverASquared;

        /**
         * We find its biggest real root. See PolynomialSolver::rootsOfCubic for comments on the method
         */
        if (double minusDiscriminant = cubicP * cubicP * cubicP + cubicQ * cubicQ; minusDiscriminant > FUZZY_ZERO) {
            double sqrtMinusDiscriminant = std::sqrt(minusDiscriminant);
            biggestRoot = std::cbrt(-cubicQ + sqrtMinusDiscriminant) + std::cbrt(-cubicQ - sqrtMinusDiscriminant);
        } else if (minusDiscriminant < -FUZZY_ZERO) {
            double sqrtMinusP = std::sqrt(-cubicP);
            double angle = std::acos(cubicQ / (cubicP * sqrtMinusP)) / 3.0;
            double cosine = sqrtMinusP * std::cos(angle);
            biggestRoot = 2 * cosine;
        } else {  // Fuzzy vanish
            if (std::abs(cubicP) < FUZZY_ZERO) {
                biggestRoot = 0.0;
            } else {
                double qOverP = cubicQ / cubicP;
                biggestRoot = (qOverP < 0.0 ? -qOverP : 2.0 * qOverP);
                // -qOverP is the double root ignored in PolynomialSolver::rootsOfCubic
            }
        }
        // Transform the root of the depressed resolvent cubic to a root of the resolvent cubic.
        biggestRoot -= 5.0 * quarticP;
    }

    double argSqrt = 1.5 * quarticP + 0.5 * biggestRoot;

    if (solveAsBiquadratic || argSqrt < FUZZY_ZERO) {
        /**
         * Exceptional case of a biquadratic equation
         */
        double quarticR =
                eOverA - 4.0 * bOverATimesdOverA + 6.0 * bOverASquared * cOverA - 3.0 * bOverASquared * bOverASquared;
        double delta = 9.0 * quarticP - quarticR;
        if (delta > FUZZY_ZERO) {
            double sqrtDelta = std::sqrt(delta);
            double r1 = -3.0 * quarticP - sqrtDelta;
            double r2 = -3.0 * quarticP + sqrtDelta;
            if (r1 > FUZZY_ZERO) {
                /**
                 * Four distinct real roots
                 */
                double sqrtR1 = std::sqrt(r1);
                double sqrtR2 = std::sqrt(r2);
                return clampToTinyVector<4>(
                        {quarticShift - sqrtR2, quarticShift - sqrtR1, quarticShift + sqrtR1, quarticShift + sqrtR2},
                        min, max);
            }
            if (r2 > FUZZY_ZERO) {
                /**
                 * Two distinct real roots and two complex roots (ignored) or one double real root (ignored)
                 */
                double sqrtR2 = std::sqrt(r2);
                return clampToTinyVector<4>({quarticShift - sqrtR2, quarticShift + sqrtR2}, min, max);
            }
        }
        /**
         * The roots are either complex or double. Ignore them all
         */
        return {};
    }

    /**
     * The roots of the depressed quartic t^4 + 6*quarticP*t^2 + 4*quarticQ*t + quarticR are the roots of the quadratic
     * equations
     *
     * P1(u) = u^2 - 2*quadB*u + 6*quarticP + biggestRoot + quarticQ / quadB
     * and
     * P2(u) = u^2 + 2*quadB*u + 6*quarticP + biggestRoot - quarticQ / quadB
     */
    double quadB = std::sqrt(argSqrt);

    double half1Delta = -4.5 * quarticP + -0.5 * biggestRoot;
    double half2Delta = quarticQ / quadB;
    if (double delta1 = half1Delta + half2Delta; delta1 > FUZZY_ZERO) {
        double sqrtDelta1 = std::sqrt(delta1);

        // Roots of P1(u)
        double r1 = quarticShift - quadB - sqrtDelta1;
        double r2 = quarticShift - quadB + sqrtDelta1;

        if (double delta2 = half1Delta - half2Delta; delta2 > FUZZY_ZERO) {

            double sqrtDelta2 = std::sqrt(delta2);
            // Roots of P2(u)
            double r3 = quarticShift + quadB - sqrtDelta2;
            double r4 = quarticShift + quadB + sqrtDelta2;

            if (std::abs(r2 - r3) < FUZZY_ZERO) {
                // Ignore double root
                return clampToTinyVector<4>({r1, r4}, min, max);
            } else {
                return clampToTinyVector<4>({r1, r2, r3, r4}, min, max);
            }
        }
        return clampToTinyVector<4>({r1, r2}, min, max);
    } else if (double delta2 = half1Delta - half2Delta; delta2 > FUZZY_ZERO) {
        double sqrtDelta2 = std::sqrt(delta2);
        return clampToTinyVector<4>({quarticShift + quadB - sqrtDelta2, quarticShift + quadB + sqrtDelta2}, min, max);
    }
    return {};
}

template <unsigned int N>
auto PolynomialSolver::clampToTinyVector(std::initializer_list<double> input, double min, double max)
        -> TinyVector<double, N> {
    static_assert(N <= 4);  // Not necessary. Should bigger N's be needed, consider doing a binary search instead
    TinyVector<double, N> res;
    std::copy_if(input.begin(), input.end(), std::back_inserter(res),
                 [min, max](double t) { return min < t && t <= max; });
    return res;
}
