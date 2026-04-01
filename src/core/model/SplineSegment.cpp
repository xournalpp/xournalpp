#include "SplineSegment.h"

#include <cmath>     // for abs
#include <iterator>  // for end

SplineSegment::SplineSegment(const Point& p, const Point& q):
        firstKnot(p), secondKnot(q), firstControlPoint(p), secondControlPoint(q) {}

SplineSegment::SplineSegment(const Point& p, const Point& fp, const Point& sp, const Point& q) {
    firstKnot = p;
    firstControlPoint = fp;
    secondControlPoint = sp;
    secondKnot = q;
}

void SplineSegment::draw(cairo_t* cr) const {
    cairo_move_to(cr, firstKnot.x, firstKnot.y);
    cairo_curve_to(cr, firstControlPoint.x, firstControlPoint.y, secondControlPoint.x, secondControlPoint.y,
                   secondKnot.x, secondKnot.y);
}

auto SplineSegment::toPointSequence(bool usePressure) const -> std::list<Point> {
    if (isFlatEnough(usePressure)) {
        return {firstKnot};
    }
    auto const& childSegments = subdivide(0.5, usePressure);
    auto left_points = childSegments.first.toPointSequence(usePressure);
    left_points.splice(end(left_points), childSegments.second.toPointSequence(usePressure));
    return left_points;
}

auto SplineSegment::subdivide(float t, bool usePressure) const -> std::pair<SplineSegment, SplineSegment> {

    Point b0 = SplineSegment::linearInterpolate(firstKnot, firstControlPoint, t);  // Same as evaluating a Bezier
    Point b1 = SplineSegment::linearInterpolate(firstControlPoint, secondControlPoint, t);
    Point b2 = SplineSegment::linearInterpolate(secondControlPoint, secondKnot, t);

    Point c0 = SplineSegment::linearInterpolate(b0, b1, t);
    Point c1 = SplineSegment::linearInterpolate(b1, b2, t);

    Point d0 = SplineSegment::linearInterpolate(c0, c1, t);  // This would be the interpolated point

    if (usePressure) {
        /**
         * Beware: with this quite imperfect formula, the pressure does not change linearly with the lengths
         */
        d0.z = t * firstKnot.z + (1 - t) * secondKnot.z;
    }

    SplineSegment firstPart = SplineSegment(firstKnot, b0, c0, d0);    // first point of each step
    SplineSegment secondPart = SplineSegment(d0, c1, b2, secondKnot);  // last point of each step
    return std::make_pair(firstPart, secondPart);
}

auto SplineSegment::linearInterpolate(const Point& p, const Point& q, float t) -> Point {
    return Point(p.x * (1 - t) + q.x * t, p.y * (1 - t) + q.y * t);
}

constexpr double FLATNESS_TOLERANCE = 1.0001;
constexpr double MIN_KNOT_DISTANCE = 0.3;
constexpr double MAX_WIDTH_VARIATION = 0.1;

auto SplineSegment::isFlatEnough(bool usePressure) const -> bool {
    double l1 = firstKnot.lineLengthTo(firstControlPoint);
    double l2 = firstControlPoint.lineLengthTo(secondControlPoint);
    double l3 = secondControlPoint.lineLengthTo(secondKnot);
    double l = firstKnot.lineLengthTo(secondKnot);
    return l < MIN_KNOT_DISTANCE || (l1 + l2 + l3 < FLATNESS_TOLERANCE * l &&
                                     ((!usePressure) || std::abs(firstKnot.z - secondKnot.z) <= MAX_WIDTH_VARIATION));
}
