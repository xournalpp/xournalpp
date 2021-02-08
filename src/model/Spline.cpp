/**
 * Xournal++
 *
 * A spline
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
/**
 * The algorithms implemented in the class Spline::SchneiderApproximater come from
 *
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 */

#include "Spline.h"

#include <cmath>
#include <numeric>

#ifdef EXTRA_CAREFUL
#include <iomanip>
#include <iostream>
#include <sstream>
#endif

Spline::Spline(const Point& firstKnot) { data.push_back(firstKnot); }

Spline::Spline(const Point& firstKnot, size_t size) {
    data.reserve(3 * size + 1);
    data.push_back(firstKnot);
}

Spline::Spline(const SplineSegment& segment) {
    data = {segment.firstKnot, segment.firstControlPoint, segment.secondControlPoint, segment.secondKnot};
}

Spline::SegmentIteratable<SplineSegment, Point> Spline::segments() {
    if (data.empty()) {
        return SegmentIteratable<SplineSegment, Point>(nullptr, nullptr);
    }
    return SegmentIteratable<SplineSegment, Point>(&data.front(), &data.back());
}

Spline::SegmentIteratable<const SplineSegment, const Point> Spline::segments() const {
    if (data.empty()) {
        return SegmentIteratable<const SplineSegment, const Point>(nullptr, nullptr);
    }
    return SegmentIteratable<const SplineSegment, const Point>(&data.front(), &data.back());
}

void Spline::addLineSegment(const Point& q) {
    const Point& p = data.back();
    data.push_back(p.relativeLineTo(q, 1.0 / 3.0));
    data.push_back(p.relativeLineTo(q, 2.0 / 3.0));
    data.push_back(q);
}

void Spline::addQuadraticSegment(const Point& cp, const Point& q) {
    data.push_back(data.back().relativeLineTo(cp, 2.0 / 3.0));
    data.push_back(q.relativeLineTo(cp, 2.0 / 3.0));
    data.push_back(q);
}

void Spline::addCubicSegment(const Point& fp, const Point& sp, const Point& q) {
    data.push_back(fp);
    data.push_back(sp);
    data.push_back(q);
}

void Spline::addCubicSegment(const MathVect3& fVelocity, const MathVect3 sVelocity, const Point& q) {
    data.push_back(fVelocity.translatePoint(data.back()));
    data.push_back(sVelocity.translatePoint(q));
    data.push_back(q);
}

void Spline::addCubicSegment(const SplineSegment& seg) {
    data.push_back(seg.firstControlPoint);
    data.push_back(seg.secondControlPoint);
    data.push_back(seg.secondKnot);
}

void Spline::setFirstKnot(const Point& p) {
    if (data.empty()) {
        data.emplace_back(p);
    } else {
        data.front() = p;
    }
}

auto Spline::getSegment(size_t index) const -> const SplineSegment& {
    return *(SplineSegment*)(data.data() + 3 * index);
}

auto Spline::getPoint(Spline::Parameter parameter) const -> Point {
    if (data.empty()) {
        g_warning("Spline::getPoint: Empty spline");
        return Point(NAN, NAN);
    }

    size_t index = parameter.index;
    double t = parameter.t;
    size_t n = size();

    if (index == 0 && t == 0.0) {
        return data.front();
    }
    if (index == n - 1 && t == 1.0) {
        return data.back();
    }

    if (index >= n || t < 0.0 || t >= 1.0) {
        g_warning("Spline::getPoint: Parameter out of range: %zu >= %zu || %f < 0.0 || %f >= 1.0", index, n, t, t);
        return Point(NAN, NAN);
    }
    return getSegment(index).getPoint(t);
}

auto Spline::cloneSection(const Spline::Parameter& lowerBound, const Spline::Parameter& upperBound) const -> Spline {

    if (upperBound.index == lowerBound.index) {
        SplineSegment segment = getSegment(lowerBound.index).getSubsegment(lowerBound.t, upperBound.t);
        return Spline(segment);
    } else {
        SplineSegment firstSegment = getSegment(lowerBound.index).subdivide(lowerBound.t).second;

        // Create and reserve memory for the clone
        Spline clone(firstSegment.firstKnot, upperBound.index - lowerBound.index + 1);
        clone.addCubicSegment(firstSegment);

        // firstControlPoint of getSegment(lowerBound.index + 1)
        auto it = data.cbegin() + 3 * (std::ptrdiff_t)lowerBound.index + 4;

        // firstControlPoint of getSegment(upperBound.index)
        auto endIt = data.cbegin() + 3 * (std::ptrdiff_t)upperBound.index + 1;

        std::copy(it, endIt, std::back_inserter(clone.data));

        SplineSegment lastSegment = getSegment(upperBound.index).subdivide(upperBound.t).first;
        clone.addCubicSegment(lastSegment);

#ifdef EXTRA_CAREFUL
        if (clone.data.size() != 3 * (upperBound.index - lowerBound.index + 1) + 1) {
            g_warning("Spline::cloneSection: wrong clone size: %zu. Indices: %zu and %zu", clone.data.size(),
                      lowerBound.index, upperBound.index);
        }
#endif
        return clone;
    }
}

auto Spline::getBoundingBox() const -> Rectangle<double> {
    const Point& firstKnot = getFirstKnot();
    Rectangle<double> result{firstKnot.x, firstKnot.y, 0.0, 0.0};
    for (auto&& segment: this->segments()) {
        result.unite(segment.getBoundingBox());
    }
    return result;
}

void Spline::toPoints(std::vector<Point>& points) const {
    if (data.empty()) {
        return;
    }
    for (auto&& segment: this->segments()) {
        segment.toPoints(points);
    }

    points.push_back(data.back());
}

auto Spline::size() const -> size_t { return data.empty() ? 0 : (data.size() - 1) / 3; }

void Spline::resize(size_t n) {
    n *= 3;
    n++;  // corresponding number of points
    if (n < data.size()) {
        data.resize(n);
    }
}

void Spline::move(double dx, double dy) {
    for (auto& p: data) {
        p.x += dx;
        p.y += dy;
    }
}

auto Spline::intersectWithRectangle(const Rectangle<double>& rectangle) const -> std::vector<Spline::Parameter> {
    if (data.empty()) {
        return {};
    }
    std::vector<Parameter> result;
    const bool startInside = data.front().isInside(rectangle);
    if (startInside) {
        result.emplace_back(0, 0.0);
    }
    size_t index = 0;
    for (auto&& seg: this->segments()) {
        Rectangle<double> box = seg.getBoundingBox();
        if (box.intersects(rectangle) || isPointOnBoundary(seg.secondKnot, rectangle)) {
            /**
             * Either the interiors meet
             * or
             * we are in the improbable situation where a knot is exactly on the boundary of the rectangle,
             * we still need to know whether the spline is crossing in or out of the rectangle.
             */
            std::vector<double> intersection = seg.intersectWithRectangle(rectangle);
            std::transform(intersection.cbegin(), intersection.cend(), std::back_inserter(result),
                           [&index](double t) { return Parameter(index, t); });
        }
        index++;
    }
    if (data.back().isInside(rectangle)) {
        size_t n = data.size() >= 4 ? size() - 1 : 0;
        result.emplace_back(n, 1.0);
    }
    /**
     * Do we need to take care of the very improbable cases where the first or last knot lie on the boundary?
     */

#ifdef EXTRA_CAREFUL
    if (result.size() % 2) {
        g_warning("Spline::intersectWithRectangle: Odd number. This should never happen");
    }
#endif

    return result;
}

auto Spline::intersectWithRectangle(const Rectangle<double>& rectangle, size_t firstIndex, size_t lastIndex) const
        -> std::vector<Spline::Parameter> {

#ifdef EXTRA_CAREFUL
    std::stringstream ss;
    ss << "Call: ** rectangle (" << rectangle.x << " ; " << rectangle.y << ") -- (" << rectangle.x + rectangle.width
       << " ; " << rectangle.y + rectangle.height << ")\n";
    ss << "      ** param: (" << firstIndex << " ; " << lastIndex << ")\n";
#endif

    std::vector<Parameter> result;
    auto inserter = std::back_inserter(result);
    size_t index = firstIndex;

    SegmentIteratable segments = this->segments();
    auto it = segments.iteratorAt(index);

    /**
     * The first (portion of a) segment
     *
     * Perform the bounding boxes tests first to save time
     */
    if (it->getCoarseBoundingBox().intersects(rectangle) && it->getBoundingBox().intersects(rectangle)) {
        std::vector<double> intersections = it->intersectWithRectangle(rectangle);

#ifdef EXTRA_CAREFUL
        ss << "I ** " << index << " : ";
        for (auto&& t: intersections) {
            ss << t << " ; ";
        }
#endif

        if (isPointOnBoundary(it->firstKnot, rectangle)) {
            /**
             * Improbable case: the segment begins on the rectangle's boundary
             */
            const Point& p = intersections.empty() ? it->secondKnot : it->getPoint(intersections.front() / 2.0);
            if (p.isInside(rectangle)) {
                /**
                 * Exceptional case: The segment begins on the rectangle's boundary and goes inwards.
                 * Add an intersection point
                 */
                result.emplace_back(index, 0.0);
#ifdef EXTRA_CAREFUL
                ss << "exc1";
                g_message("Spline::intersectWithRectangle: Exceptional case 1");
            } else {
                ss << "imp1";
                g_message("Spline::intersectWithRectangle: Improbable case 1");
#endif
            }

        } else {
            if (it->firstKnot.isInside(rectangle)) {
                /**
                 * The spline starts in the rectangle. Add a fake intersection parameter
                 */
                result.emplace_back(index, 0.0);
#ifdef EXTRA_CAREFUL
                ss << "start in";
#endif
            }
        }
#ifdef EXTRA_CAREFUL
        ss << "\n";
#endif

        std::transform(intersections.begin(), intersections.end(), inserter,
                       [index](double v) { return Parameter(index, v); });
    }

    auto endIt = segments.iteratorAt(lastIndex + 1);
    it++;  // We already took care of the first segment
    index++;

    for (; it != endIt; it++, index++) {
        /**
         * Should we store the bounding boxes somewhere (and where? SplineSegment? Spline? Stroke? EraseableStroke?)
         * Only test with getCoarseBoundingBox()?
         */
        if ((it->getCoarseBoundingBox().intersects(rectangle) && it->getBoundingBox().intersects(rectangle)) ||
            isPointOnBoundary(it->secondKnot, rectangle)) {
            /**
             * Either the interiors meet
             * or
             * we are in the improbable situation where a knot is exactly on the boundary of the rectangle,
             * Either way, we need to know whether the spline is crossing in or out of the rectangle.
             */
            std::vector<double> intersection = it->intersectWithRectangle(rectangle);
            std::transform(intersection.cbegin(), intersection.cend(), inserter,
                           [&index](double t) { return Parameter(index, t); });
#ifdef EXTRA_CAREFUL
            ss << "I ** " << index << " : ";
            for (auto&& t: intersection) {
                ss << t << " ; ";
            }
            ss << "\n";
        } else {
            ss << "O ** " << index << "\n";
#endif
        }
    }

    if (result.size() % 2) {
        const SplineSegment& seg = it[-1];
        if (seg.secondKnot.isInside(rectangle)) {
            /**
             * The spline ends in the rectangle (not on the boundary). Add a fake intersection parameter
             */
#ifdef EXTRA_CAREFUL
            ss << "end in";
#endif
            result.emplace_back(lastIndex, 1.0);
        } else {
            /**
             * The only possibility:
             * The segment ends on the rectangle's boundary, coming from outside
             * Drop this last intersection point
             */
#ifdef EXTRA_CAREFUL
            ss << "\n--- Result: ";
            bool even = true;
            for (auto&& p: result) {
                ss << "(" << p.index << " ; " << p.t << ")";
                ss << (even ? " to " : "\n");
                even = !even;
            }
            std::cout << ss.str() << "\n";
            g_message("Spline::intersectWithRectangle: Improbable case 2. May also be a bug.");
#endif
            result.pop_back();
        }
    }
    return result;
}

void Spline::debugPrint() const {
    /**
     * 3D svg-like dump of the spline
     */
    //     g_message("Spline %zu", (uint64_t)this);
    //     if (!this->data.empty()) {
    //         const Point& firstKnot = data.front();
    //         g_message("M %f %f %f", firstKnot.x, firstKnot.y, firstKnot.z);
    //         for (auto&& seg: *this) {
    //             g_message("C %f %f %f, %f %f %f, %f %f %f", seg.firstControlPoint.x, seg.firstControlPoint.y,
    //                       seg.firstControlPoint.z, seg.secondControlPoint.x, seg.secondControlPoint.y,
    //                       seg.secondControlPoint.z, seg.secondKnot.x, seg.secondKnot.y, seg.secondKnot.z);
    //         }
    //     }
    if (!this->data.empty()) {
        const Point& firstKnot = data.front();
        for (auto&& seg: this->segments()) {
            printf("%f ; %f ; %f ; %f\n%f ; %f ; %f ; %f\n\n", seg.firstKnot.x, seg.firstControlPoint.x,
                   seg.secondControlPoint.x, seg.secondKnot.x, seg.firstKnot.y, seg.firstControlPoint.y,
                   seg.secondControlPoint.y, seg.secondKnot.y);
        }
    }
}

/**
 * Catmull-Rom interpolation
 */
auto Spline::getCentripetalCatmullRomInterpolation(const std::vector<Point>& points) -> Spline {
    /**
     * Reference: https://qroph.github.io/2018/07/30/smooth-paths-using-catmull-rom-splines.html
     */

    size_t pointCount = points.size();
    if (pointCount == 0) {
        g_warning("Catmull-Rom interpolation on 0 points. This should never happen");
        return Spline(Point(-1, -1));
    }

    Spline result(points[0]);

    if (pointCount == 1) {
        return result;
    }

    if (pointCount == 2) {
        result.addLineSegment(points[1]);
        return result;
    }

    MathVect3 v01(points[0], points[1]);
    MathVect3 v12(points[1], points[2]);

    CatmullRomComputer crc(v01, v12);

    /**
     * Heuristic: the first spline segment is quadratic.
     */
    MathVect3 u = -crc.t01 * crc.m;
    result.addCubicSegment(u + 2.0 / 3.0 * v01, u, points[1]);

    MathVect3 fVelocity = crc.t12 * crc.m;

    auto it1 = std::next(points.begin(), 2);
    auto it2 = std::next(it1);
    while (it2 != points.end()) {
        crc.addStep(MathVect3(*it1, *it2));
        result.addCubicSegment(fVelocity, -crc.t01 * crc.m, *it1);
        fVelocity = crc.t12 * crc.m;
        it1++;
        it2++;
    }

    /**
     * Heuristic: the last spline segment is quadratic.
     */
    result.addCubicSegment(fVelocity, fVelocity - 2.0 / 3.0 * crc.getLastVector(), points.back());

    return result;
}

Spline::CatmullRomComputer::CatmullRomComputer(const MathVect3& u, const MathVect3& v): head(0) {
    t12 = std::pow(u.dx * u.dx + u.dy * u.dy + u.dz * u.dz, 0.25);  // = sqrt(u.norm())
    diff[0] = u;
    addStep(v);
}

void Spline::CatmullRomComputer::addStep(const MathVect3& u) {
    t01 = t12;
    t12 = std::pow(u.dx * u.dx + u.dy * u.dy + u.dz * u.dz, 0.25);  // = sqrt(u.norm())
    const size_t oldhead = head;
    head = (head + 1) % 2;
    diff[head] = u;
    double inverse = 1.0 / (t01 + t12);
    m = ((1.0 / t12 - inverse) / 3.0) * diff[head] + ((1.0 / t01 - inverse) / 3.0) * diff[oldhead];
}

auto Spline::CatmullRomComputer::getLastVector() -> const MathVect3& { return diff[head]; }


/**
 * Schneider approximation
 */
auto Spline::getSchneiderApproximation(const std::vector<Point>& points) -> Spline {
    if (points.empty()) {
        g_warning("Spline::getSchneiderApproximation called on empty vector. This should never happen");
        return Spline(Point(0, 0));
    }
    SchneiderApproximater approximator(points);
#ifdef EXTRA_CAREFUL
    approximator.printStats();
#endif
    return approximator.getSpline();
}

Spline::SchneiderApproximater::SchneiderApproximater(const std::vector<Point>& points):
        points(points), spline(points.front()), chordLength{0.0} {
    size_t maxIndex = points.size() - 1;
    if (maxIndex > 0) {
        /**
         * Fill the chordLength
         */
        chordLength.reserve(points.size());
        double length = 0.0;
        for (auto it1 = points.cbegin(), it2 = std::next(points.cbegin()); it2 != points.cend(); it1++, it2++) {
            length += it2->lineLengthTo(*it1);
            chordLength.push_back(length);
        }

        /**
         * Compute the tangent vectors
         */
        MathVect3 firstTangentVector(points[0], points[1]);
        MathVect3 secondTangentVector(points[maxIndex], points[maxIndex - 1]);
        firstTangentVector.normalize();
        secondTangentVector.normalize();

#ifdef EXTRA_CAREFUL
        //         g_message("points.size() = %zu, chordLength.size() = %zu", points.size(), chordLength.size());
        if (firstTangentVector.dx == 0.0 && firstTangentVector.dy == 0.0) {  // && firstTangentVector.dz == 0.0) {
            g_warning("Spline::SchneiderApproximater::SchneiderApproximater: first tangent vector is 0");
        }
        if (secondTangentVector.dx == 0.0 && secondTangentVector.dy == 0.0) {  // && secondTangentVector.dz == 0.0) {
            g_warning("Spline::SchneiderApproximater::SchneiderApproximater: second tangent vector is 0");
        }
#endif
        fitCubic(0, firstTangentVector, secondTangentVector, maxIndex);
    }
}

void Spline::SchneiderApproximater::printStats() {
    size_t nbPoints = points.size();
    size_t nbSegments = spline.size();
    totalNbSegments += nbSegments;
    totalNbPoints += nbPoints;
    g_message("Schneider: %3zu pts => %3zu segs. Total %4zu pts => %4zu segs (~ %4zu pts)", nbPoints, nbSegments,
              totalNbPoints, totalNbSegments, (totalNbSegments * 7 + 3) / 3);
}
size_t Spline::SchneiderApproximater::totalNbPoints = 0;
size_t Spline::SchneiderApproximater::totalNbSegments = 0;


auto Spline::SchneiderApproximater::getSpline() -> Spline { return spline; }

void Spline::SchneiderApproximater::fitCubic(size_t lowerIndex, const MathVect3& firstTangentVector,
                                             const MathVect3& secondTangentVector, size_t upperIndex) {

#ifdef EXTRA_CAREFUL
    if (lowerIndex >= upperIndex || upperIndex >= points.size()) {
        g_warning("Spline::SchneiderApproximater::fitCubic called on invalid indices");
        return;
    }
#endif

    const Point& secondKnot = points[upperIndex];

    if (upperIndex == lowerIndex + 1) {
        /**
         * Use Wu-Barsky heuristic
         * TODO try it out with Catmull-Rom
         */
        double scale = points[lowerIndex].lineLengthTo(secondKnot) / 3.0;

#ifdef EXTRA_CAREFUL
        if ((!points[lowerIndex].equalsPos(spline.getLastKnot())) || points[lowerIndex].z != spline.getLastKnot().z) {
            g_warning("Spline::SchneiderApproximater::fitCubic: Wrong last knot!");
        }
#endif

        spline.addCubicSegment(scale * firstTangentVector, scale * secondTangentVector, secondKnot);
        return;
    }

    // Declared before the scope starts
    size_t middleIndex;
    MathVect3 middleTangentVector;
    {  // Scope of segmentFitter
        SingleSegmentFitter segmentFitter(
                firstTangentVector, secondTangentVector, std::next(points.cbegin(), (long)lowerIndex),
                std::next(points.cbegin(), (long)upperIndex), getStandardParametrization(lowerIndex, upperIndex));

        bool parametrizationValid = true;
        double lastError = ITERATION_ERROR;

        for (size_t i = 0; i < NUMBER_OF_ITERATIONS; i++) {
            segmentFitter.findBestCubicSegment();
            double maxError = segmentFitter.computeMaxError();
            if (maxError < ERROR && parametrizationValid) {
                //                 g_message("   OK!!");
                spline.addCubicSegment(segmentFitter.getFirstVelocity(), segmentFitter.getSecondVelocity(), secondKnot);
                return;
            }
            if (maxError >= lastError) {
                /**
                 * The error is either to big or getting bigger
                 */
                break;
            }
            lastError = maxError;
            parametrizationValid = segmentFitter.reparametrize();
            //             g_message("  - reparametrize");
        }

        /**
         * Fitting failed. Split at worst point and try fitting both sides separately
         */
        auto worstPoint = segmentFitter.getWorstPoint();
        middleTangentVector = getMiddleTangent(worstPoint);
        middleIndex = (size_t)std::distance(points.cbegin(), worstPoint);
    }  // End of scope of segmentFitter

    fitCubic(lowerIndex, firstTangentVector, -middleTangentVector, middleIndex);
    fitCubic(middleIndex, middleTangentVector, secondTangentVector, upperIndex);
}

std::vector<double> Spline::SchneiderApproximater::getStandardParametrization(size_t lowerIndex, size_t upperIndex) {
    auto beginIt = std::next(chordLength.cbegin(), (long)lowerIndex);
    auto endIt = std::next(chordLength.cbegin(), (long)upperIndex + 1);
    double reference = *beginIt;
    double length = chordLength[upperIndex] - reference;

    std::vector<double> result;
    result.reserve(upperIndex - lowerIndex + 1);

    if (fuzzyVanish(length)) {  // (length == 0.0) {
        g_warning("Spline::SchneiderApproximater::getStandardParametrization: length zero");
        result.resize(upperIndex - lowerIndex + 1, 0.0);
    } else {
        std::transform(beginIt, endIt, std::back_inserter(result),
                       [reference, length](double u) { return (u - reference) / length; });
    }
    return result;
}

MathVect3 Spline::SchneiderApproximater::getMiddleTangent(std::vector<Point>::const_iterator it) {
    MathVect3 result(it[-1], it[1]);
    result.normalize();
    return result;
}


Spline::SchneiderApproximater::SingleSegmentFitter::SingleSegmentFitter(const MathVect3& fTgt, const MathVect3& sTgt,
                                                                        std::vector<Point>::const_iterator pointsBegin,
                                                                        std::vector<Point>::const_iterator pointsLast,
                                                                        std::vector<double> parametrization):
        fTgt(fTgt),
        sTgt(sTgt),
        diff(*pointsBegin, *pointsLast),
        sp_fTgt_sTgt(MathVect3::scalarProduct(fTgt, sTgt)),
        sp_fTgt_diff(MathVect3::scalarProduct(fTgt, diff)),
        sp_sTgt_diff(MathVect3::scalarProduct(sTgt, diff)),
        squaredNormDiff(MathVect3::scalarProduct(diff, diff)),
        fTgtZero(fTgt.dx == 0.0 &&
                 fTgt.dy == 0.0),  // && fTgt.dz == 0.0),  // No need to fuzzy compare (see fTgt.normalize())
        sTgtZero(sTgt.dx == 0.0 &&
                 sTgt.dy == 0.0),  // && sTgt.dz == 0.0),  // No need to fuzzy compare (see sTgt.normalize())
        pointsBegin(pointsBegin),
        pointsEnd(std::next(pointsLast)),
        worstPoint(std::next(pointsBegin, std::distance(pointsBegin, pointsEnd) / 2)),
        parametrization(parametrization) {
#ifdef EXTRA_CAREFUL
    if (parametrization.size() != std::distance(pointsBegin, pointsEnd)) {
        g_warning("Spline::SchneiderApproximater::SingleSegmentFitter::SingleSegmentFitter: wrong parametrization size "
                  "%zu vs %zu",
                  parametrization.size(), std::distance(pointsBegin, pointsEnd));
    }
#endif
}

void Spline::SchneiderApproximater::SingleSegmentFitter::findBestCubicSegment() {
    /**
     * Find the best possible (single) spline segment fitting the data points, with prescribed endpoints and with
     * initial and final velocity vectors proportional to the provided tangent vectors
     *
     * The fitted parameters are the norms of the two velocity vectors.
     */
    double C[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double X[2] = {0.0, 0.0};

    const Point firstPoint = *(pointsBegin);

    auto itPts = pointsBegin;
    auto itParam = parametrization.cbegin();
    while (itPts != pointsEnd) {
        /**
         * Fill the matrices. Compared to Schneider's original code
         *      https://github.com/erich666/GraphicsGems/blob/master/gems/FitCurves.c
         * this has been optimized by:
         *  * computing most scalar products only once
         *  * multiplying by the scale factors after taking the scalar products
         *  * expressing the points' coordinates relatively to the first point so the computation of X has less terms
         */
        double u = *itParam;
        double t = 1 - u;
        double b = 3 * u * t;
        double b1 = b * t;
        double b2 = b * u;
        double b23 = b2 + u * u * u;

        C[0][0] += b1 * b1;
        C[0][1] += b1 * b2;
        C[1][1] += b2 * b2;

        MathVect3 step(firstPoint, *itPts);

        X[0] += b1 * (MathVect3::scalarProduct(fTgt, step) - (b23 * sp_fTgt_diff));
        X[1] += b2 * (MathVect3::scalarProduct(sTgt, step) - (b23 * sp_sTgt_diff));

        itPts++;
        itParam++;
    }
    C[0][0] = (fTgtZero ? 0.0 : C[0][0]);
    C[0][1] *= sp_fTgt_sTgt;
    C[1][0] = C[0][1];
    C[1][1] = (sTgtZero ? 0.0 : C[1][1]);
#ifdef EXTRA_CAREFUL
    if (itParam != parametrization.cend()) {
        g_warning("Spline::SchneiderApproximater::SingleSegmentFitter::findBestCubicSegment: Not the end of "
                  "parametrization");
    }
#endif

    /**
     * Will contain the result: best norms of the velocity vectors
     */
    double alpha0;
    double alpha1;

    double detC = C[0][0] * C[1][1] - C[0][1] * C[1][0];
    if (!fuzzyVanish(detC)) {  // detC != 0.0) {
        /**
         * Use Kramer's rule to solve the system
         */
        alpha0 = (C[1][1] * X[0] - C[0][1] * X[1]) / detC;
        alpha1 = (C[0][0] * X[1] - C[1][0] * X[0]) / detC;

    } else {
        /**
         * The system is under-determined. Try assuming alpha0 == alpha1.
         */
        double c = C[0][0] + C[0][1];
        if (!fuzzyVanish(c)) {  //(c != 0.0) {
            alpha0 = X[0] / c;
        } else {
            c = C[1][0] + C[1][1];
            if (!fuzzyVanish(c)) {  //(c != 0.0) {
                alpha0 = X[1] / c;
            } else {
                alpha0 = 0.0;
            }
        }
        alpha1 = alpha0;
    }

    // TODO: Should this test be fuzzy?
    if (alpha0 <= 0.0 || alpha1 <= 0.0 || alpha0 * sp_fTgt_diff - alpha1 * sp_sTgt_diff > squaredNormDiff) {
        /**
         * Fallback to the Wu-Barsky heuristic
         */
        alpha0 = alpha1 = sqrt(squaredNormDiff) / 3.0;
    }

    fVelocity = alpha0 * fTgt;
    sVelocity = alpha1 * sTgt;
}

double Spline::SchneiderApproximater::SingleSegmentFitter::computeMaxError() {
    double maxDistance = 0.0;

    errors.clear();
    errors.reserve(parametrization.size());

    auto itPts = pointsBegin;
    auto itParam = parametrization.cbegin();
    while (itPts != pointsEnd) {
        double u = *itParam;
        double t = 1 - u;
        double ut3 = u * t * 3;
        /**
         * Compute the error vector. To simplify the formulae, the origin is set to firstKnot = *pointBegin.
         */
        MathVect3 error = (t * ut3) * fVelocity + (ut3 * u) * sVelocity + ((1 + 2 * t) * u * u) * diff -
                          MathVect3(*pointsBegin, *itPts);

        double dist = MathVect3::scalarProduct(error, error);  // Squared norm
        if (dist > maxDistance) {
            maxDistance = dist;
            worstPoint = itPts;
        }
        errors.push_back(error);

        itParam++;
        itPts++;
    }
    return maxDistance;
}

bool Spline::SchneiderApproximater::SingleSegmentFitter::reparametrize() {

    auto errorIt = errors.cbegin();
    for (double& u: parametrization) {
        double t = 1 - u;
        double threeUMinusTwo = 3.0 * u - 2.0;
        double oneMinusThreeU = -1.0 - threeUMinusTwo;
        double twoU = 2.0 * u;

        /**
         * Compute the derivatives for Newton-Raphson iteration
         */
        MathVect3 thirdQprimeOfU =
                t * oneMinusThreeU * fVelocity + (twoU * t) * diff + (-u * threeUMinusTwo) * sVelocity;
        MathVect3 sixthQsecondOfU = threeUMinusTwo * fVelocity + (1 - twoU) * diff + oneMinusThreeU * sVelocity;

        double denominator = 3.0 * MathVect3::scalarProduct(thirdQprimeOfU, thirdQprimeOfU) +
                             2.0 * MathVect3::scalarProduct(*errorIt, sixthQsecondOfU);

        if (fuzzyVanish(denominator)) {  // denominator == 0.0) {
            g_warning("Spline::SchneiderApproximater::SingleSegmentFitter::reparametrize: zero denominator");
            errorIt++;
            continue;
        }
        double numerator = MathVect3::scalarProduct(*errorIt, thirdQprimeOfU);
        u -= numerator / denominator;
        errorIt++;
    }

    auto it1 = parametrization.cbegin();
    auto it2 = std::next(parametrization.cbegin());
    while (it2 != parametrization.cend()) {
        if (*it2 <= *it1) {
            /**
             * The reparametrization messed the order up. The new parametrization is invalid.
             */
            return false;
        }
        it1++;
        it2++;
    }
    return true;
}

MathVect3 Spline::SchneiderApproximater::SingleSegmentFitter::getFirstVelocity() const { return fVelocity; }

MathVect3 Spline::SchneiderApproximater::SingleSegmentFitter::getSecondVelocity() const { return sVelocity; }

std::vector<Point>::const_iterator Spline::SchneiderApproximater::SingleSegmentFitter::getWorstPoint() const {
    return worstPoint;
}
