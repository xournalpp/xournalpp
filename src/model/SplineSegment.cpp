#include "SplineSegment.h"

#include <cmath>
#include <numeric>

/**
 * PartialSplineSegment
 */
PartialSplineSegment::PartialSplineSegment(const Point& p, const Point& q) {
    secondKnot = q;
    firstControlPoint = {2.0 / 3.0 * p.x + 1.0 / 3.0 * q.x, 2.0 / 3.0 * p.y + 1.0 / 3.0 * q.y,
                         2.0 / 3.0 * p.z + 1.0 / 3.0 * q.z};
    secondControlPoint = {1.0 / 3.0 * p.x + 2.0 / 3.0 * q.x, 1.0 / 3.0 * p.y + 2.0 / 3.0 * q.y,
                          1.0 / 3.0 * p.z + 2.0 / 3.0 * q.z};
}

PartialSplineSegment::PartialSplineSegment(const Point& fp, const Point& sp, const Point& q) {
    secondKnot = q;
    firstControlPoint = fp;
    secondControlPoint = sp;
}

PartialSplineSegment::PartialSplineSegment(const Point& p, const MathVect3& fVelocity, const MathVect3 sVelocity,
                                           const Point& q) {
    secondKnot = q;
    firstControlPoint = fVelocity.translatePoint(p);
    secondControlPoint = sVelocity.translatePoint(q);
}

auto PartialSplineSegment::subdivide(const Point& firstKnot, double t) const
        -> std::pair<PartialSplineSegment, PartialSplineSegment> {
    Point b0 = firstKnot.relativeLineTo(firstControlPoint, t);  // Same as evaluating a Bézier
    Point b1 = firstControlPoint.relativeLineTo(secondControlPoint, t);
    Point b2 = secondControlPoint.relativeLineTo(secondKnot, t);

    Point c0 = b0.relativeLineTo(b1, t);
    Point c1 = b1.relativeLineTo(b2, t);

    Point d0 = c0.relativeLineTo(c1, t);  // This would be the interpolated point

    PartialSplineSegment firstPart(b0, c0, d0);           // first point of each step
    PartialSplineSegment secondPart(c1, b2, secondKnot);  // last point of each step
    return std::make_pair(firstPart, secondPart);
}

void PartialSplineSegment::toPoints(const Point& firstKnot, std::vector<Point>& points) const {
    const double l1 = firstKnot.lineLengthTo(firstControlPoint);
    const double l2 = firstControlPoint.lineLengthTo(secondControlPoint);
    const double l3 = secondControlPoint.lineLengthTo(secondKnot);
    const double l = firstKnot.lineLengthTo(secondKnot);
    const double widthChange = std::abs(firstKnot.z - secondKnot.z);
    
    if (l < MIN_KNOT_DISTANCE || (l1 + l2 + l3 < FLATNESS_TOLERANCE * l && widthChange < MAX_WIDTH_CHANGE)) {
        points.push_back(secondKnot);
        return;
    }
    
    auto const& childSegments = subdivide(firstKnot, 0.5);
    childSegments.first.toPoints(firstKnot, points);
    childSegments.second.toPoints(childSegments.first.secondKnot, points);
}

auto PartialSplineSegment::getX(const Point& firstKnot, double t) const -> double {
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    return u * u * u * firstKnot.x + u * ut3 * firstControlPoint.x + t * ut3 * secondControlPoint.x + t * t * t * secondKnot.x;
}

auto PartialSplineSegment::getY(const Point& firstKnot, double t) const -> double {
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    return u * u * u * firstKnot.y + u * ut3 * firstControlPoint.y + t * ut3 * secondControlPoint.y + t * t * t * secondKnot.y;
}

auto PartialSplineSegment::getPoint(const Point& firstKnot, double t) const -> Point {
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    double B0 = u * u * u;
    double B1 = u * ut3;
    double B2 = t * ut3;
    double B3 = t * t * t;
    return Point(B0 * firstKnot.x + B1 * firstControlPoint.x + B2 * secondControlPoint.x + B3 * secondKnot.x,
                 B0 * firstKnot.y + B1 * firstControlPoint.y + B2 * secondControlPoint.y + B3 * secondKnot.y,
                 B0 * firstKnot.z + B1 * firstControlPoint.z + B2 * secondControlPoint.z + B3 * secondKnot.z);
}

auto PartialSplineSegment::rootsOfPolynomialEquation(double a, double b, double c) -> std::vector<double> {
    if (a == 0.0) {
        /**
         * The equation is linear
         */
        if (b != 0.0) {
            return {-c / (2.0 * b)};
        }
        return {};
    }

    /**
     * Reduced discriminant of the polynomial equation
     */
    double Delta = b * b - a * c;
    
    if (Delta > 0) {
        double sqrtDelta = sqrt(Delta);
        return {(-b + sqrtDelta) / a, (-b - sqrtDelta) / a};
    }
    
    if (Delta == 0.0) {
        return {-b / a};
    }
    
    return {};
}

auto PartialSplineSegment::getBoundingBox(const Point& firstKnot) const -> Rectangle<double> {
    /**
     * Compute the extrema of the spline coordinates using the closed mathematical formula
     */
    
    /**
     * The x coordinates first
     */
    double minX;
    double maxX;
    if (firstKnot.x < secondKnot.x) {
        minX = firstKnot.x;
        maxX = secondKnot.x;
    } else {
        maxX = firstKnot.x;
        minX = secondKnot.x;
    }
    
    /**
     * Polynomial coefficients of x'(t) / 3 = a * t^2 + 2 * b * t + c
     */
    double a = secondKnot.x + 3 * (firstControlPoint.x - secondControlPoint.x)  - firstKnot.x;
    double b = secondControlPoint.x - 2 * firstControlPoint.x + firstKnot.x;
    double c = firstControlPoint.x - firstKnot.x;
    std::vector<double> roots = rootsOfPolynomialEquation(a, b, c);
    
    for (double t: roots) {
        if (t > 0.0 && t < 1.0) {
            double x = getX(firstKnot, t);
            if (x >= maxX) {
                maxX = x;
            } else {
                minX = std::min(x, minX);
            }
        }
    }
    
    /**
     * Now the y coordinates
     */
    double minY;
    double maxY;
    if (firstKnot.y < secondKnot.y) {
        minY = firstKnot.y;
        maxY = secondKnot.y;
    } else {
        maxY = firstKnot.y;
        minY = secondKnot.y;
    }
    
    /**
     * Polynomial coefficients of y'(t) / 3 = a * t^2 + 2 * b * t + c
     */
    a = secondKnot.y + 3 * (firstControlPoint.y - secondControlPoint.y)  - firstKnot.y;
    b = secondControlPoint.y - 2 * firstControlPoint.y + firstKnot.y;
    c = firstControlPoint.y - firstKnot.y;
    roots = rootsOfPolynomialEquation(a, b, c);
    
    for (double t: roots) {
        if (t > 0.0 && t < 1.0) {
            double y = getY(firstKnot, t);
            if (y >= maxY) {
                maxY = y;
            } else {
                minY = std::min(y, minY);
            }
        }
    }
    
    return Rectangle(minX, minY, maxX - minX, maxY - minY);
}


/**
 * SplineSegment
 */
SplineSegment::SplineSegment(const Point& p, const Point& q): PartialSplineSegment(p, q), firstKnot(p) {}

SplineSegment::SplineSegment(const Point& p, const Point& fp, const Point& sp, const Point& q):
        PartialSplineSegment(fp, sp, q), firstKnot(p) {}

SplineSegment::SplineSegment(Point& p, const PartialSplineSegment& partial):
        PartialSplineSegment(partial), firstKnot(p) {}


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

auto SplineSegment::subdivide(float t) const -> std::pair<SplineSegment, SplineSegment> {

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

auto SplineSegment::isFlatEnough() const -> bool {
    double l1 = firstKnot.lineLengthTo(firstControlPoint);
    double l2 = firstControlPoint.lineLengthTo(secondControlPoint);
    double l3 = secondControlPoint.lineLengthTo(secondKnot);
    double l = firstKnot.lineLengthTo(secondKnot);
    
    double widthChange = std::abs(firstKnot.z - secondKnot.z);
    return  l < MIN_KNOT_DISTANCE || (l1 + l2 + l3 < FLATNESS_TOLERANCE * l && widthChange < MAX_WIDTH_CHANGE);
}
