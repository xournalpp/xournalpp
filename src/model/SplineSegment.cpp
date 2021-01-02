#include "SplineSegment.h"

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

auto SplineSegment::toPointSequence() const -> std::list<Point> {
    if (isFlatEnough()) {
        return {firstKnot};
    }
    auto const& childSegments = subdivide(0.5);
    auto left_points = childSegments.first.toPointSequence();
    left_points.splice(end(left_points), childSegments.second.toPointSequence());
    return left_points;
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
