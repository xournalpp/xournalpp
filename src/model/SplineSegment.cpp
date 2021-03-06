#include "SplineSegment.h"

#include <cmath>
#include <numeric>

#include "Element.h"
#include "Interval.h"
#include "PolynomialSolver.h"

SplineSegment::SplineSegment(const Point& p, const Point& fp, const Point& sp, const Point& q):
        firstKnot(p), firstControlPoint(fp), secondControlPoint(sp), secondKnot(q) {}

void SplineSegment::draw(cairo_t* cr) const {
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

auto SplineSegment::getCoarseBoundingBox() const -> Rectangle<double> {
    Interval<double> knotsX = Interval<double>::getInterval(firstKnot.x, secondKnot.x);
    Interval<double> intervalX = Interval<double>::getInterval(firstControlPoint.x, secondControlPoint.x);
    intervalX.envelop(knotsX);
    Interval<double> knotsY = Interval<double>::getInterval(firstKnot.y, secondKnot.y);
    Interval<double> intervalY = Interval<double>::getInterval(firstControlPoint.y, secondControlPoint.y);
    intervalY.envelop(knotsY);
    return Rectangle<double>(intervalX.min, intervalY.min, intervalX.length(), intervalY.length());
}

auto SplineSegment::getBoundingBox() const -> Rectangle<double> {
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
    std::vector<double> roots = PolynomialSolver::rootsOfQuadratic(a, b, c, 0.0, 1.0);

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

    return Rectangle(intervalX.min, intervalY.min, intervalX.length(), intervalY.length());
}

auto SplineSegment::intersectWithHorizontalLine(double lineY) const -> std::vector<double> {
    const double a = secondKnot.y - firstKnot.y + 3.0 * (firstControlPoint.y - secondControlPoint.y);
    const double b = firstKnot.y + secondControlPoint.y - 2.0 * firstControlPoint.y;
    const double c = firstControlPoint.y - firstKnot.y;
    const double d = firstKnot.y - lineY;
    return PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);
}

auto SplineSegment::intersectWithVerticalLine(double lineX) const -> std::vector<double> {
    const double a = secondKnot.x - firstKnot.x + 3.0 * (firstControlPoint.x - secondControlPoint.x);
    const double b = firstKnot.x + secondControlPoint.x - 2.0 * firstControlPoint.x;
    const double c = firstControlPoint.x - firstKnot.x;
    const double d = firstKnot.x - lineX;
    return PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);
}

auto SplineSegment::intersectWithRectangle(const Rectangle<double>& rectangle) const -> std::vector<double> {
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
        std::vector<double> roots1 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

        d -= rectangle.height;
        std::vector<double> roots2 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

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
        std::vector<double> roots1 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

        d -= rectangle.width;
        std::vector<double> roots2 = PolynomialSolver::rootsOfCubic(a, b, c, d, 0.0, 1.0);

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
