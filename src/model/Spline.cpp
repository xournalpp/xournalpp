#include "Spline.h"

#include <cmath>
#include <numeric>

#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"

#include "SplineSchneiderApproximator.h"

#ifdef EXTRA_CAREFUL
#include <iomanip>
#include <iostream>
#include <sstream>
#endif

Spline& Spline::operator=(const std::vector<Point>& pts) {
    this->data = pts;
    return *this;
}

Spline::Spline(const Point& firstKnot) { data.push_back(firstKnot); }

Spline::Spline(const Point& firstKnot, size_t size) {
    data.reserve(3 * size + 1);
    data.push_back(firstKnot);
}

Spline::Spline(const SplineSegment& segment) {
    data = {segment.firstKnot, segment.firstControlPoint, segment.secondControlPoint, segment.secondKnot};
}

Spline::Spline(ObjectInputStream& in) {
    in.readObject("Spline");

    Point* p{};
    int count{};
    in.readData(reinterpret_cast<void**>(&p), &count);
    this->data = std::vector<Point>{p, p + count};
    g_free(p);

    in.endObject();
}

void Spline::serialize(ObjectOutputStream& out) const {
    out.writeObject("Spline");
    out.writeData(this->data.data(), this->data.size(), sizeof(Point));
    out.endObject();
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

auto Spline::getFirstKnot() const -> const Point& { return this->data.front(); }

auto Spline::getLastKnot() const -> const Point& { return this->data.back(); }

auto Spline::getSegment(size_t index) const -> const SplineSegment& {
    return *(SplineSegment*)(data.data() + 3 * index);
}

auto Spline::getPoint(const Parameter& parameter) const -> Point {
    if (data.empty()) {
        g_warning("Spline::getPoint: Empty spline");
        return Point(NAN, NAN);
    }

    size_t index = parameter.index;
    double t = parameter.t;
    size_t n = nbSegments();

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

auto Spline::clone() const -> std::unique_ptr<Path> {
    std::unique_ptr<Spline> clone = std::make_unique<Spline>(*this);
    //     clone.data.reserve(this->data.size());
    //     std::copy(this->data.cbegin(), this->data.cend(), std::back_inserter(clone.data));
    return clone;
}

auto Spline::cloneSection(const Parameter& lowerBound, const Parameter& upperBound) const -> std::unique_ptr<Path> {

    if (upperBound.index == lowerBound.index) {
        SplineSegment segment = getSegment(lowerBound.index).getSubsegment(lowerBound.t, upperBound.t);
        return std::make_unique<Spline>(segment);
    } else {
        SplineSegment firstSegment = getSegment(lowerBound.index).subdivide(lowerBound.t).second;

        // Create and reserve memory for the clone
        std::unique_ptr<Spline> clone =
                std::make_unique<Spline>(firstSegment.firstKnot, upperBound.index - lowerBound.index + 1);
        clone->addCubicSegment(firstSegment);

        // firstControlPoint of getSegment(lowerBound.index + 1)
        auto it = data.cbegin() + 3 * (std::ptrdiff_t)lowerBound.index + 4;

        // firstControlPoint of getSegment(upperBound.index)
        auto endIt = data.cbegin() + 3 * (std::ptrdiff_t)upperBound.index + 1;

        std::copy(it, endIt, std::back_inserter(clone->data));

        SplineSegment lastSegment = getSegment(upperBound.index).subdivide(upperBound.t).first;
        clone->addCubicSegment(lastSegment);

#ifdef EXTRA_CAREFUL
        if (clone->data.size() != 3 * (upperBound.index - lowerBound.index + 1) + 1) {
            g_warning("Spline::cloneSection: wrong clone size: %zu. Indices: %zu and %zu", clone->data.size(),
                      lowerBound.index, upperBound.index);
        }
#endif
        return clone;
    }
}

auto Spline::getThinBoundingBox() const -> Rectangle<double> {
    if (data.empty()) {
        return {0.0, 0.0, 0.0, 0.0};
    }
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

auto Spline::nbSegments() const -> size_t { return data.empty() ? 0 : (data.size() - 1) / 3; }

void Spline::resize(size_t n) {
    n *= 3;
    n++;  // corresponding number of points
    if (n < data.size()) {
        data.resize(n);
    }
}

auto Spline::intersectWithRectangle(const Rectangle<double>& rectangle) const -> std::vector<Parameter> {
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
        size_t n = data.size() >= 4 ? nbSegments() - 1 : 0;
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
        -> std::vector<Parameter> {

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
