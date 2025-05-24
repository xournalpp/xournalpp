#include "SplineSegment.h"

#include <algorithm>  // for max, min, copy, merge
#include <array>      // for array
#include <cmath>      // for abs
#include <iterator>   // for end
#include <optional>   // for optional

#include "util/Interval.h"
#include "util/PolynomialSolver.h"
#include "util/Range.h"

#include "Element.h"
#include "MathVect.h"

SplineSegment::SplineSegment(const Point& p, const Point& fp, const Point& sp, const Point& q):
        firstKnot(p), firstControlPoint(fp), secondControlPoint(sp), secondKnot(q) {}

SplineSegment::SplineSegment(const Point& p, const MathVect3& fTgt, const MathVect3& sTgt, const Point& q):
        firstKnot(p),
        firstControlPoint(fTgt.translatePoint(p)),
        secondControlPoint(sTgt.translatePoint(q)),
        secondKnot(q) {}

SplineSegment::SplineSegment(const Point& p):
        firstKnot(p), firstControlPoint(p), secondControlPoint(p), secondKnot(p) {}

SplineSegment::SplineSegment(const Point& p, const Point& q):
        firstKnot(p), firstControlPoint(p), secondControlPoint(q), secondKnot(q) {}

SplineSegment::SplineSegment(const Point& p, const Point& cp, const Point& q):
        firstKnot(p),
        firstControlPoint(p.relativeLineTo(cp, 2.0 / 3.0)),
        secondControlPoint(q.relativeLineTo(cp, 2.0 / 3.0)),
        secondKnot(q) {}


void SplineSegment::toCairo(cairo_t* cr) const {
    cairo_move_to(cr, firstKnot.x, firstKnot.y);
    cairo_curve_to(cr, firstControlPoint.x, firstControlPoint.y, secondControlPoint.x, secondControlPoint.y,
                   secondKnot.x, secondKnot.y);
}

void SplineSegment::toPoints(std::vector<Point>& points) const {
    if (isFlatEnough()) {
        points.push_back(firstKnot);
        return;
    }
    auto const& childSegments = subdivide(0.5);
    childSegments.first.toPoints(points);
    childSegments.second.toPoints(points);
}

double SplineSegment::length() const {
    if (isFlatEnough()) {
        return firstKnot.lineLengthTo(secondKnot);
    }
    auto const& childSegments = subdivide(0.5);
    return childSegments.first.length() + childSegments.second.length();
}

void SplineSegment::toParametrizedPoints(std::vector<ParametrizedPoint>& points, double start, double end) const {
    if (isFlatEnough()) {
        points.emplace_back(firstKnot, start);
        return;
    }
    auto const& childSegments = subdivide(0.5);
    const double middle = 0.5 * (start + end);
    childSegments.first.toParametrizedPoints(points, start, middle);
    childSegments.second.toParametrizedPoints(points, middle, end);
}

auto SplineSegment::subdivide(double t) const -> std::pair<SplineSegment, SplineSegment> {

    Point b0 = firstKnot.relativeLineTo(firstControlPoint, t);  // Same as evaluating a Bézier
    Point b1 = firstControlPoint.relativeLineTo(secondControlPoint, t);
    Point b2 = secondControlPoint.relativeLineTo(secondKnot, t);

    Point c0 = b0.relativeLineTo(b1, t);
    Point c1 = b1.relativeLineTo(b2, t);

    Point d0 = c0.relativeLineTo(c1, t);  // This would be the interpolated point

    SplineSegment firstPart = SplineSegment(firstKnot, b0, c0, d0);    // first point of each step
    SplineSegment secondPart = SplineSegment(d0, c1, b2, secondKnot);  // last point of each step
    return std::make_pair(firstPart, secondPart);
}

auto SplineSegment::getSubsegment(double tMin, double tMax) const -> SplineSegment {
    return subdivide(tMax).first.subdivide(getParameterInFirstSubsegment(tMin, tMax)).second;
}

auto SplineSegment::isFlatEnough() const -> bool {
    double l1 = firstKnot.lineLengthTo(firstControlPoint);
    double l2 = firstControlPoint.lineLengthTo(secondControlPoint);
    double l3 = secondControlPoint.lineLengthTo(secondKnot);
    double l = firstKnot.lineLengthTo(secondKnot);
    double widthChange = std::abs(firstKnot.z - secondKnot.z);
    return l < MIN_KNOT_DISTANCE || (l1 + l2 + l3 < FLATNESS_TOLERANCE * l && widthChange < MAX_WIDTH_CHANGE);
}

auto SplineSegment::getX(double t) const -> double {
    if (t == 0.0) {
        return firstKnot.x;
    }
    if (t == 1.0) {
        return secondKnot.x;
    }
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    return u * u * u * firstKnot.x + u * ut3 * firstControlPoint.x + t * ut3 * secondControlPoint.x +
           t * t * t * secondKnot.x;
}

auto SplineSegment::getY(double t) const -> double {
    if (t == 0.0) {
        return firstKnot.y;
    }
    if (t == 1.0) {
        return secondKnot.y;
    }
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    return u * u * u * firstKnot.y + u * ut3 * firstControlPoint.y + t * ut3 * secondControlPoint.y +
           t * t * t * secondKnot.y;
}

auto SplineSegment::getZ(double t) const -> double {
    if (t == 0.0) {
        return firstKnot.z;
    }
    if (t == 1.0) {
        return secondKnot.z;
    }
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    return u * u * u * firstKnot.z + u * ut3 * firstControlPoint.z + t * ut3 * secondControlPoint.z +
           t * t * t * secondKnot.z;
}

auto SplineSegment::getPoint(double t) const -> Point {
    if (t == 0.0) {
        return firstKnot;
    }
    if (t == 1.0) {
        return secondKnot;
    }
    double u = 1.0 - t;
    double ut3 = u * t * 3.0;
    double B0 = u * u * u;
    double B1 = u * ut3;
    double B2 = t * ut3;
    double B3 = t * t * t;
    return Point(B0 * firstKnot.x + B1 * firstControlPoint.x + B2 * secondControlPoint.x + B3 * secondKnot.x,
                 B0 * firstKnot.y + B1 * firstControlPoint.y + B2 * secondControlPoint.y + B3 * secondKnot.y,
                 B0 * firstKnot.z + B1 * firstControlPoint.z + B2 * secondControlPoint.z + B3 * secondKnot.z);
}

auto SplineSegment::getCoarseBoundingBox() const -> Range {
    Interval<double> knotsX = Interval<double>::getInterval(firstKnot.x, secondKnot.x);
    Interval<double> intervalX = Interval<double>::getInterval(firstControlPoint.x, secondControlPoint.x);
    intervalX.envelop(knotsX);
    Interval<double> knotsY = Interval<double>::getInterval(firstKnot.y, secondKnot.y);
    Interval<double> intervalY = Interval<double>::getInterval(firstControlPoint.y, secondControlPoint.y);
    intervalY.envelop(knotsY);

    return Range(intervalX.min, intervalY.min, intervalX.max, intervalY.max);
}

auto SplineSegment::getThinBoundingBox() const -> Range {
    /**
     * Compute the extrema of the spline coordinates using the closed mathematical formula
     */

    /**
     * The x coordinates first
     */
    //  Polynomial coefficients of x'(t) / 3 = a * t^2 + 2 * b * t + c
    double a = secondKnot.x + 3 * (firstControlPoint.x - secondControlPoint.x) - firstKnot.x;
    double b = secondControlPoint.x - 2 * firstControlPoint.x + firstKnot.x;
    double c = firstControlPoint.x - firstKnot.x;
    auto roots = PolynomialSolver::rootsOfQuadratic(a, b, c, 0.0, 1.0);

    Interval<double> intervalX = Interval<double>::getInterval(firstKnot.x, secondKnot.x);
    for (double t: roots) {
        double x = getX(t);
        intervalX.envelop(x);
    }

    /**
     * Now the y coordinates
     */
    //  Polynomial coefficients of y'(t) / 3 = a * t^2 + 2 * b * t + c
    a = secondKnot.y + 3 * (firstControlPoint.y - secondControlPoint.y) - firstKnot.y;
    b = secondControlPoint.y - 2 * firstControlPoint.y + firstKnot.y;
    c = firstControlPoint.y - firstKnot.y;
    roots = PolynomialSolver::rootsOfQuadratic(a, b, c, 0.0, 1.0);

    Interval<double> intervalY = Interval<double>::getInterval(firstKnot.y, secondKnot.y);
    for (double t: roots) {
        double y = getY(t);
        intervalY.envelop(y);
    }

    return Range(intervalX.min, intervalY.min, intervalX.max, intervalY.max);
}

auto SplineSegment::getWidthUpperbound() const -> double {
    Interval<double> widthInterval = Interval<double>::getInterval(firstKnot.z, secondKnot.z);
    widthInterval.envelop(firstControlPoint.z);
    widthInterval.envelop(secondControlPoint.z);
    return widthInterval.max;
}

auto SplineSegment::getMaximalWidth() const -> double {
    /**
     * Compute the maximal width of the spline using the closed mathematical formula
     */
    //  Polynomial coefficients of z'(t) / 3 = a * t^2 + 2 * b * t + c
    double a = secondKnot.z + 3 * (firstControlPoint.z - secondControlPoint.z) - firstKnot.z;
    double b = secondControlPoint.z - 2 * firstControlPoint.z + firstKnot.z;
    double c = firstControlPoint.z - firstKnot.z;
    auto roots = PolynomialSolver::rootsOfQuadratic(a, b, c, 0.0, 1.0);

    double maxWidth = std::max(firstKnot.z, secondKnot.z);
    for (double t: roots) {
        maxWidth = std::max(maxWidth, getZ(t));
    }
    return maxWidth;
}

auto SplineSegment::getThickBoundingBox() const -> Range {
    /**
     * Coefficients for width
     */
    const double halfFirstZ = 0.5 * firstKnot.z;
    const double halfSecondZ = 0.5 * secondKnot.z;
    const double aWidth = halfSecondZ + 1.5 * (firstControlPoint.z - secondControlPoint.z) - halfFirstZ;
    const double bWidth = 0.5 * (secondControlPoint.z + firstKnot.z) - firstControlPoint.z;
    const double cWidth = 0.5 * (firstControlPoint.z - firstKnot.z);

    /**
     * The x coordinates first
     */
    //  Polynomial coefficients of x'(t) / 3 = a * t^2 + 2 * b * t + c
    double a = secondKnot.x + 3 * (firstControlPoint.x - secondControlPoint.x) - firstKnot.x;
    double b = secondControlPoint.x - 2 * firstControlPoint.x + firstKnot.x;
    double c = firstControlPoint.x - firstKnot.x;

    double minX = std::min(firstKnot.x - halfFirstZ, secondKnot.x - halfSecondZ);
    auto roots = PolynomialSolver::rootsOfQuadratic(a - aWidth, b - bWidth, c - cWidth, 0.0, 1.0);
    for (double t: roots) {
        Point p = getPoint(t);
        minX = std::min(minX, p.x - 0.5 * p.z);
    }

    double maxX = std::max(firstKnot.x + halfFirstZ, secondKnot.x + halfSecondZ);
    roots = PolynomialSolver::rootsOfQuadratic(a + aWidth, b + bWidth, c + cWidth, 0.0, 1.0);
    for (double t: roots) {
        Point p = getPoint(t);
        maxX = std::max(maxX, p.x + 0.5 * p.z);
    }
    /**
     * Now the y coordinates
     */
    //  Polynomial coefficients of y'(t) / 3 = a * t^2 + 2 * b * t + c
    a = secondKnot.y + 3 * (firstControlPoint.y - secondControlPoint.y) - firstKnot.y;
    b = secondControlPoint.y - 2 * firstControlPoint.y + firstKnot.y;
    c = firstControlPoint.y - firstKnot.y;

    double minY = std::min(firstKnot.y - halfFirstZ, secondKnot.y - halfSecondZ);
    roots = PolynomialSolver::rootsOfQuadratic(a - aWidth, b - bWidth, c - cWidth, 0.0, 1.0);
    for (double t: roots) {
        Point p = getPoint(t);
        minY = std::min(minY, p.y - 0.5 * p.z);
    }

    double maxY = std::max(firstKnot.y + halfFirstZ, secondKnot.y + halfSecondZ);
    roots = PolynomialSolver::rootsOfQuadratic(a + aWidth, b + bWidth, c + cWidth, 0.0, 1.0);
    for (double t: roots) {
        Point p = getPoint(t);
        maxY = std::max(maxY, p.y + 0.5 * p.z);
    }

    return Range(minX, minY, maxX, maxY);
}

auto SplineSegment::intersectWithHorizontalLine(double lineY) const -> TinyVector<double, 3> {
    const double a = secondKnot.y - firstKnot.y + 3.0 * (firstControlPoint.y - secondControlPoint.y);
    const double b = firstKnot.y + secondControlPoint.y - 2.0 * firstControlPoint.y;
    const double c = firstControlPoint.y - firstKnot.y;
    const double d = firstKnot.y - lineY;
    return PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);
}

auto SplineSegment::intersectWithVerticalLine(double lineX) const -> TinyVector<double, 3> {
    const double a = secondKnot.x - firstKnot.x + 3.0 * (firstControlPoint.x - secondControlPoint.x);
    const double b = firstKnot.x + secondControlPoint.x - 2.0 * firstControlPoint.x;
    const double c = firstControlPoint.x - firstKnot.x;
    const double d = firstKnot.x - lineX;
    return PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);
}

auto SplineSegment::intersectWithRectangle(const xoj::util::Rectangle<double>& rectangle) const -> std::vector<double> {
    if (this->getThinBoundingBox().intersect(Range(rectangle)).empty()) {
        return {};
    }
    /**
     * Find where the spline segment enters or leaves the horizontal ribbon:
     *          rectangle.y < y < rectangle.y + rectangle.height
     */
    std::vector<double> horizontalIntersections;
    {  // Scope for populating horizontalIntersections
        double a = secondKnot.y - firstKnot.y + 3.0 * (firstControlPoint.y - secondControlPoint.y);
        double b = firstKnot.y + secondControlPoint.y - 2.0 * firstControlPoint.y;
        double c = firstControlPoint.y - firstKnot.y;
        double d = firstKnot.y - rectangle.y;
        auto roots1 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

        d -= rectangle.height;
        auto roots2 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

        std::merge(roots1.begin(), roots1.end(), roots2.begin(), roots2.end(),
                   std::back_inserter(horizontalIntersections));
    }

    /**
     * Find where the spline segment enters or leaves the vertical ribbon:
     *          rectangle.x < x < rectangle.x + rectangle.width
     */
    std::vector<double> verticalIntersections;
    {  // Scope for populating verticalIntersections
        double a = secondKnot.x - firstKnot.x + 3.0 * (firstControlPoint.x - secondControlPoint.x);
        double b = firstKnot.x + secondControlPoint.x - 2.0 * firstControlPoint.x;
        double c = firstControlPoint.x - firstKnot.x;
        double d = firstKnot.x - rectangle.x;
        auto roots1 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

        d -= rectangle.width;
        auto roots2 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

        std::merge(roots1.begin(), roots1.end(), roots2.begin(), roots2.end(),
                   std::back_inserter(verticalIntersections));
    }

    /**
     * Compute the intersections of the spline segment with the edges of the rectangle
     */
    std::vector<double> result;
    bool insideX = firstKnot.x >= rectangle.x && firstKnot.x <= rectangle.x + rectangle.width;
    bool insideY = firstKnot.y >= rectangle.y && firstKnot.y <= rectangle.y + rectangle.height;

    auto itX = verticalIntersections.cbegin();
    auto itY = horizontalIntersections.cbegin();
    while (itX != verticalIntersections.cend() && itY != horizontalIntersections.cend()) {
        if (*itX < *itY) {
            insideX = !insideX;
            if (insideY) {
                result.push_back(*itX);
            }
            *itX++;
        } else {
            insideY = !insideY;
            if (insideX) {
                result.push_back(*itY);
            }
            *itY++;
        }
    }

    // At most one of the following std::copy has a non-empty range
    if (insideY) {
        std::copy(itX, verticalIntersections.cend(), std::back_inserter(result));
    }
    if (insideX) {
        std::copy(itY, horizontalIntersections.cend(), std::back_inserter(result));
    }

    return result;
}

bool SplineSegment::isTailInSelection(ShapeContainer* container, bool assumeSecondKnotIn) const {
    if (!assumeSecondKnotIn && !container->contains(this->secondKnot.x, this->secondKnot.y)) {
        return false;
    }
    if (!container->contains(this->firstControlPoint.x, this->firstControlPoint.y) ||
        !container->contains(this->secondControlPoint.x, this->secondControlPoint.y)) {
        if (this->isFlatEnough()) {
            return true;
        }
        auto const& childSegments = subdivide(0.5);
        return childSegments.first.isTailInSelection(container, false) &&
               childSegments.second.isTailInSelection(container, true);
    }
    return true;
}

std::pair<double, double> SplineSegment::closestPointTo(const Point& p) const {
    /**
     * Find a minimum to the degree 6 polynomial computing the distance from p to the segment
     *
     * Even if the segment has already been converted into line segments (with toPoints),
     * this is still 4x faster than working with those line segments
     */

    /**
     * A = firstKnot
     * B = firstControlPoint
     * C = secondControlPoint
     * D = secondKnot
     * P = p
     */
    MathVect2 PA(p, firstKnot);
    MathVect2 AB(firstKnot, firstControlPoint);
    MathVect2 BCminusAB(firstKnot.x - 2.0 * firstControlPoint.x + secondControlPoint.x,
                        firstKnot.y - 2.0 * firstControlPoint.y + secondControlPoint.y);
    MathVect2 ABminus2BCplusCD(-firstKnot.x + 3.0 * (firstControlPoint.x - secondControlPoint.x) + secondKnot.x,
                               -firstKnot.y + 3.0 * (firstControlPoint.y - secondControlPoint.y) + secondKnot.y);

    double a0 = PA.squaredNorm();
    double a1 = 6.0 * MathVect2::scalarProduct(PA, AB);
    double a2 = 6.0 * MathVect2::scalarProduct(PA, BCminusAB) + 9.0 * AB.squaredNorm();
    double a3 = 2.0 * MathVect2::scalarProduct(PA, ABminus2BCplusCD) + 18.0 * MathVect2::scalarProduct(AB, BCminusAB);
    double a4 = 6.0 * MathVect2::scalarProduct(AB, ABminus2BCplusCD) + 9.0 * BCminusAB.squaredNorm();
    double a5 = 6.0 * MathVect2::scalarProduct(BCminusAB, ABminus2BCplusCD);
    double a6 = ABminus2BCplusCD.squaredNorm();

    PolynomialSolver::Polynomial<6> squaredDistance({a6, a5, a4, a3, a2, a1, a0});
    PolynomialSolver::PolynomialSolver<5> solver(squaredDistance.getDerivative());
    auto roots = solver.findRoots(0.0, 1.0);

    double min = a0;
    double t = 0.0;
    if (double d = a0 + a1 + a2 + a3 + a4 + a5 + a6; min > d) {
        min = d;
        t = 1.0;
    }
    for (double r: roots) {
        if (double d = squaredDistance.evaluate(r); min > d) {
            min = d;
            t = r;
        }
    }
    return {t, min};
}

double SplineSegment::squaredDistanceToHull(const Point& p) const {
    /**
     * We compute the squared distance between p and the 6 possible segments based on two points of
     * {firstKnot, firstControlPoint, secondControlPoint, secondKnot}
     */
    MathVect2 u1(p, firstKnot);
    MathVect2 u2(p, firstControlPoint);
    MathVect2 u3(p, secondControlPoint);
    MathVect2 u4(p, secondKnot);
    double min;

    // v = MathVect2(firstKnot, firstControlPoint);
    if (MathVect2 v = u2 - u1; v.isZero()) {
        min = u1.squaredNorm();
    } else {
        /**
         * t is the relative linear coordinate of the projection of p onto the line passing through firstKnot and
         * firstControlPoint
         */
        double t = -MathVect2::scalarProduct(u1, v) / v.squaredNorm();
        min = (t <= 0.0 ? u1.squaredNorm() : (t >= 1.0 ? u2.squaredNorm() : (u1 + t * v).squaredNorm()));
    }

    // v = MathVect2(firstKnot, secondControlPoint);
    if (MathVect2 v = u3 - u1; !v.isZero()) {
        double t = -MathVect2::scalarProduct(u1, v) / v.squaredNorm();
        if (t > 0.0) {
            double m = (t >= 1.0 ? u3.squaredNorm() : (u1 + t * v).squaredNorm());
            min = std::min(m, min);
        }
    }

    // v = MathVect2(firstKnot, secondKnot);
    if (MathVect2 v = u4 - u1; !v.isZero()) {
        double t = -MathVect2::scalarProduct(u1, v) / v.squaredNorm();
        if (t > 0.0) {
            double m = (t >= 1.0 ? u4.squaredNorm() : (u1 + t * v).squaredNorm());
            min = std::min(m, min);
        }
    }

    // v = MathVect2(firstControlPoint, secondControlPoint);
    if (MathVect2 v = u3 - u2; !v.isZero()) {
        double t = -MathVect2::scalarProduct(u2, v) / v.squaredNorm();
        if (t > 0.0 && t < 1.0) {
            min = std::min(min, (u2 + t * v).squaredNorm());
        }
    }

    // v = MathVect2(firstControlPoint, secondKnot);
    if (MathVect2 v = u4 - u2; !v.isZero()) {
        double t = -MathVect2::scalarProduct(u2, v) / v.squaredNorm();
        if (t > 0.0 && t < 1.0) {
            min = std::min(min, (u2 + t * v).squaredNorm());
        }
    }

    // v = MathVect2(secondControlPoint, secondKnot);
    if (MathVect2 v = u4 - u3; !v.isZero()) {
        double t = -MathVect2::scalarProduct(u3, v) / v.squaredNorm();
        if (t > 0.0 && t < 1.0) {
            min = std::min(min, (u3 + t * v).squaredNorm());
        }
    }
    return min;
}

bool SplineSegment::isNegligible() { return firstKnot.lineLengthTo(secondKnot) < MIN_KNOT_DISTANCE; }

std::pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>> SplineSegment::getLeftHalfTangent()
        const {
    return std::make_pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>>(
            this->firstKnot, this->firstControlPoint);
}

std::pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>> SplineSegment::getRightHalfTangent()
        const {
    return std::make_pair<std::reference_wrapper<const Point>, std::reference_wrapper<const Point>>(
            this->secondControlPoint, this->secondKnot);
}
