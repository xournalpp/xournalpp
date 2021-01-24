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
    return u * u * u * firstKnot.x + u * ut3 * firstControlPoint.x + t * ut3 * secondControlPoint.x +
           t * t * t * secondKnot.x;
}

auto PartialSplineSegment::getY(const Point& firstKnot, double t) const -> double {
    double u = 1 - t;
    double ut3 = u * t * 3.0;
    return u * u * u * firstKnot.y + u * ut3 * firstControlPoint.y + t * ut3 * secondControlPoint.y +
           t * t * t * secondKnot.y;
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
    double a = secondKnot.x + 3 * (firstControlPoint.x - secondControlPoint.x) - firstKnot.x;
    double b = secondControlPoint.x - 2 * firstControlPoint.x + firstKnot.x;
    double c = firstControlPoint.x - firstKnot.x;
    std::vector<double> roots = rootsOfQuadraticEquation(a, b, c);

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
    a = secondKnot.y + 3 * (firstControlPoint.y - secondControlPoint.y) - firstKnot.y;
    b = secondControlPoint.y - 2 * firstControlPoint.y + firstKnot.y;
    c = firstControlPoint.y - firstKnot.y;
    roots = rootsOfQuadraticEquation(a, b, c);

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

auto PartialSplineSegment::intersectWithHorizontalLine(const Point& firstKnot, double lineY) const
        -> std::vector<double> {
    double a = secondKnot.y - firstKnot.y + 3.0 * (firstControlPoint.y - secondControlPoint.y);
    double b = firstKnot.y + secondControlPoint.y - 2.0 * firstControlPoint.y;
    double c = firstControlPoint.y - firstKnot.y;
    double d = firstKnot.y - lineY;
    std::vector<double> roots = rootsOfCubicEquation(a, b, c, d);
    
    std::string msg = "Horizontal\nPoints: (" + 
    std::to_string(firstKnot.x) + ";" + std::to_string(firstKnot.y) + ")  (" +
    std::to_string(firstControlPoint.x) + ";" + std::to_string(firstControlPoint.y) + ")  (" +
    std::to_string(secondControlPoint.x) + ";" + std::to_string(secondControlPoint.y) + ")  (" +
    std::to_string(secondKnot.x) + ";" + std::to_string(secondKnot.y) + ")\n"
    "Coeff: " + std::to_string(a) + " ; " + std::to_string(b) + " ; " + std::to_string(c) + " ; " + std::to_string(d) + "\nRoots: ";
    for (auto&& val: roots) {
        msg += std::to_string(val) + " ";
    }
    g_message("%s", msg.c_str());
    
    std::vector<double> result;
    std::remove_copy_if(roots.cbegin(), roots.cend(), std::back_inserter(result),
                        [](double value) { return value > 1.0 || value < 0.0; });
    return result;
}

auto PartialSplineSegment::intersectWithVerticalLine(const Point& firstKnot, double lineX) const
        -> std::vector<double> {
    double a = secondKnot.x - firstKnot.x + 3.0 * (firstControlPoint.x - secondControlPoint.x);
    double b = firstKnot.x + secondControlPoint.x - 2.0 * firstControlPoint.x;
    double c = firstControlPoint.x - firstKnot.x;
    double d = firstKnot.x - lineX;
    std::vector<double> roots = rootsOfCubicEquation(a, b, c, d);
    
    g_message("firstKnot: %f %f", firstKnot.x, firstKnot.y);
    
    std::string msg = "Vertical\nPoints: (" + 
    std::to_string(firstKnot.x) + ";" + std::to_string(firstKnot.y) + ")  (" +
    std::to_string(firstControlPoint.x) + ";" + std::to_string(firstControlPoint.y) + ")  (" +
    std::to_string(secondControlPoint.x) + ";" + std::to_string(secondControlPoint.y) + ")  (" +
    std::to_string(secondKnot.x) + ";" + std::to_string(secondKnot.y) + ")\n"
    "Coeff: " + std::to_string(a) + "*t^3 + 3*" + std::to_string(b) + "*t^2 + 3*" + std::to_string(c) + "*t + " + std::to_string(d) + "\nRoots: ";
    for (auto&& val: roots) {
        msg += std::to_string(val) + " ";
    }
    g_message("%s", msg.c_str());
    
    std::vector<double> result;
    std::remove_copy_if(roots.cbegin(), roots.cend(), std::back_inserter(result),
                        [](double value) { return value > 1.0 || value < 0.0; });
    return result;
}

auto PartialSplineSegment::rootsOfQuadraticEquation(double a, double b, double c) -> std::vector<double> {
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
        // Sort the two roots from smallest to biggest
        double PMsqrtDelta = (a > 0.0 ? sqrt(Delta) : -sqrt(Delta));
        return {(-b - PMsqrtDelta) / a, (-b + PMsqrtDelta) / a};
    }

    if (Delta == 0.0) {
        return {-b / a};
    }

    return {};
}

auto PartialSplineSegment::rootsOfCubicEquation(double a, double b, double c, double d) -> std::vector<double> {
    /**
     * Solve the equation a*t^3 + 3*b*t^2 + 3*c*t + d
     * See https://en.wikipedia.org/wiki/Cubic_equation for the methods used here.
     */
    if (a == 0.0) {
        return rootsOfQuadraticEquation(3.0 * b, 1.5 * c, d);
    }

    /**
     * The equivalent depressed equation u^3 + 3 * p * u + 2 * q = 0 is obtain by the change of variable u = t + b / a
     *
     * Coefficients of the depressed equation
     */
    double bOverA = b / a;
    double bOverASquared = bOverA * bOverA;
    double cOverA = c / a;
    double p = cOverA - bOverASquared;
    double q = 0.5 * d / a + bOverA * (bOverASquared - 1.5 * cOverA);
    
    g_message("p = %f q = %f", p,q);
    
    

    /**
     * Discriminant
     */
    double minusDiscriminant = p * p * p + q * q;
    if (minusDiscriminant > 0.0) {
        /**
         * One real solution, two complex ones (which we ignore)
         * Use Cardano's formula
         */
        double sqrtMinusDiscriminant = sqrt(minusDiscriminant);
        return {-bOverA + cbrt(-q + sqrtMinusDiscriminant) + cbrt(-q - sqrtMinusDiscriminant)};
    }
    if (minusDiscriminant == 0.0) {
        /**
         * Three real solutions, twice the same
         */
        if (p == 0) {
            /**
             * A triple solution. We don't care for the multiplicity
             */
            return {-bOverA};
        }
        /**
         * Ignore the solution of multiplicity 2:
         * the spline is tangent to the line WITHOUT crossing it
         */
        return {-bOverA - 2 * q / p};
        /**
         * Nb: q / p = - cbrt(q) because minusDiscriminant == 0
         * This trick is used because it (probably) reduces the computational cost
         */
    }
    /**
     * minusDiscriminant < 0.0:
     * Three distinct real solutions
     * Use the trigonometric solutions
     */
    double sqrtMinusP = sqrt(-p);
    double angle = acos(q / (p * sqrtMinusP)) / 3.0;  // [0, M_PI_3]
    double cosine = sqrtMinusP * cos(angle);          // sqrtMinusP * [0.5, 1]
    double sine = sqrt(3) * sqrtMinusP * sin(angle);  // 3 / 2 * sqrtMinusP * [0, 1]
    return {-cosine - sine - bOverA, -cosine + sine - bOverA, 2 * cosine - bOverA};

    //  sqrtMinusP < 2 cosine < 2 sqrtMinusP
    //  -sqrtMinusP < sine - cosine < sqrtMinusP
    //  -2.5 * sqrtMinusP < -sine - cosine < -0.5 * sqrtMinusP
    //  -sine - cosine < sine - cosine
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
    return l < MIN_KNOT_DISTANCE || (l1 + l2 + l3 < FLATNESS_TOLERANCE * l && widthChange < MAX_WIDTH_CHANGE);
}
