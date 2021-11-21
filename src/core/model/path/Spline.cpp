#include "Spline.h"

#include <algorithm>
#include <cmath>

#include "model/Element.h"
#include "util/Assert.h"
#include "util/SmallVector.h"
#include "util/serializing/ObjectInputStream.h"
#include "util/serializing/ObjectOutputStream.h"

#include "IntersectWithPaddedBoxTemplate.h"
#include "PiecewiseLinearPath.h"

Spline::Spline(const std::vector<Point>& vector) { data = vector; }
Spline::Spline(std::vector<Point>&& vector) { data = std::move(vector); }

Spline& Spline::operator=(const std::vector<Point>& vector) {
    data = vector;
    return *this;
}
Spline& Spline::operator=(std::vector<Point>&& vector) {
    data = std::move(vector);
    return *this;
}

Spline::Spline(const Point& firstKnot) { data.push_back(firstKnot); }

Spline::Spline(const Point& firstKnot, size_t size) {
    data.reserve(SEGMENT_SIZE * size + 1);
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

Spline::Spline(MakeEllipsePlaceholder, const Point& center, double radiusX, double radiusY) {
    /**
     * Length of the velocity vectors of a spline segment approximating a quarter of a (unit) circle
     * With this length, the error (the max distance between the spline and the circle) is smaller than 3e-4
     */
    constexpr double TANGENT_LENGTH = (M_SQRT2 - 1.0) * 4.0 / 3.0;

    this->data.reserve(SEGMENT_SIZE * 4 + 1);
    this->setFirstKnot(Point(center.x + radiusX, center.y));
    this->addCubicSegment(Point(center.x + radiusX, center.y + radiusY * TANGENT_LENGTH),
                          Point(center.x + radiusX * TANGENT_LENGTH, center.y + radiusY),
                          Point(center.x, center.y + radiusY));
    this->addCubicSegment(Point(center.x - radiusX * TANGENT_LENGTH, center.y + radiusY),
                          Point(center.x - radiusX, center.y + radiusY * TANGENT_LENGTH),
                          Point(center.x - radiusX, center.y));
    this->addCubicSegment(Point(center.x - radiusX, center.y - radiusY * TANGENT_LENGTH),
                          Point(center.x - radiusX * TANGENT_LENGTH, center.y - radiusY),
                          Point(center.x, center.y - radiusY));
    this->addCubicSegment(Point(center.x + radiusX * TANGENT_LENGTH, center.y - radiusY),
                          Point(center.x + radiusX, center.y - radiusY * TANGENT_LENGTH),
                          Point(center.x + radiusX, center.y));
}

void Spline::serialize(ObjectOutputStream& out) const {
    out.writeObject("Spline");
    out.writeData(this->data.data(), this->data.size(), sizeof(Point));
    out.endObject();
}

Path::SegmentIteratable<SplineSegment> Spline::segments() {
    if (data.empty()) {
        return SegmentIteratable<SplineSegment>(nullptr, nullptr);
    }
    xoj_assert(data.size() % SEGMENT_SIZE == 1);
    return SegmentIteratable<SplineSegment>(&data.front(), &data.back());
}

Path::SegmentIteratable<const SplineSegment> Spline::segments() const {
    if (data.empty()) {
        return SegmentIteratable<const SplineSegment>(nullptr, nullptr);
    }
    xoj_assert(data.size() % SEGMENT_SIZE == 1);
    return SegmentIteratable<const SplineSegment>(&data.front(), &data.back());
}

void Spline::addLineSegment(const Point& q) {
    xoj_assert(!data.empty());
    const Point& p = data.back();
    data.push_back(p.relativeLineTo(q, 1.0 / 3.0));
    data.push_back(p.relativeLineTo(q, 2.0 / 3.0));
    data.push_back(q);
}

void Spline::addQuadraticSegment(const Point& cp, const Point& q) {
    xoj_assert(!data.empty());
    data.push_back(data.back().relativeLineTo(cp, 2.0 / 3.0));
    data.push_back(q.relativeLineTo(cp, 2.0 / 3.0));
    data.push_back(q);
}

void Spline::addCubicSegment(const Point& fp, const Point& sp, const Point& q) {
    xoj_assert(!data.empty());
    data.push_back(fp);
    data.push_back(sp);
    data.push_back(q);
}

void Spline::addCubicSegment(const MathVect3& fVelocity, const MathVect3 sVelocity, const Point& q) {
    xoj_assert(!data.empty());
    data.push_back(fVelocity.translatePoint(data.back()));
    data.push_back(sVelocity.translatePoint(q));
    data.push_back(q);
}

void Spline::addCubicSegment(const SplineSegment& seg) {
    xoj_assert(!data.empty());
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

void Spline::replaceLastSegment(const SplineSegment& seg) {
    xoj_assert(this->nbSegments() >= 1);
    SplineSegment& lastSegment = *(this->segments().end() - 1);
    lastSegment.firstControlPoint = seg.firstControlPoint;
    lastSegment.secondControlPoint = seg.secondControlPoint;
    lastSegment.secondKnot = seg.secondKnot;
}

auto Spline::getFirstKnot() const -> const Point& { return this->data.front(); }

auto Spline::getLastKnot() const -> const Point& { return this->data.back(); }

auto Spline::getSegment(size_t index) const -> const SplineSegment& {
    xoj_assert((data.size() % SEGMENT_SIZE == 1) && (index * SEGMENT_SIZE + SEGMENT_SIZE + 1 <= data.size()));
    return *(SplineSegment*)(data.data() + SEGMENT_SIZE * index);
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

    if (index >= n || t < 0.0 || t > 1.0) {
        g_warning("Spline::getPoint: Parameter out of range: %zu >= %zu || %f < 0.0 || %f > 1.0", index, n, t, t);
        return Point(NAN, NAN);
    }
    return getSegment(index).getPoint(t);
}

auto Spline::clone() const -> std::unique_ptr<Path> {
    std::unique_ptr<Spline> clone = std::make_unique<Spline>(*this);
    return clone;
}

auto Spline::cloneSection(const SubSection& section) const -> std::unique_ptr<Path> {

    if (section.min.index == section.max.index) {
        SplineSegment segment = getSegment(section.min.index).getSubsegment(section.min.t, section.max.t);
        if (segment.isFlatEnough()) {
            return std::make_unique<PiecewiseLinearPath>(segment.firstKnot, segment.secondKnot);
        }
        return std::make_unique<Spline>(segment);
    } else {
        // Drop first and/or last segments if they are too tiny
        SplineSegment firstSegment = getSegment(section.min.index).subdivide(section.min.t).second;
        bool dropFirst = firstSegment.isNegligible();

        SplineSegment lastSegment = getSegment(section.max.index).subdivide(section.max.t).first;
        bool dropLast = lastSegment.isNegligible();

        // Create and reserve memory for the clone
        std::unique_ptr<Spline> clone =
                std::make_unique<Spline>(firstSegment.firstKnot, section.max.index - section.min.index + 1 -
                                                                         (dropLast ? 1 : 0) - (dropFirst ? 1 : 0));

        if (!dropFirst) {
            clone->addCubicSegment(firstSegment);
        }

        // firstControlPoint of getSegment(section.min.index + 1)
        auto it = data.cbegin() + SEGMENT_SIZE * (std::ptrdiff_t)section.min.index + SEGMENT_SIZE + 1;

        // firstControlPoint of getSegment(section.max.index)
        auto endIt = data.cbegin() + SEGMENT_SIZE * (std::ptrdiff_t)section.max.index + 1;

        std::copy(it, endIt, std::back_inserter(clone->data));

        if (!dropLast) {
            clone->addCubicSegment(lastSegment);
        }

        xoj_assert(clone->data.size() % SEGMENT_SIZE == 1);
        xoj_assert(clone->nbSegments() ==
                   section.max.index - section.min.index + 1 - (dropLast ? 1 : 0) - (dropFirst ? 1 : 0));
        return clone;
    }
}

std::unique_ptr<Path> Spline::cloneCircularSectionOfClosedPath(const Path::Parameter& startParam,
                                                               const Path::Parameter& endParam) const {

    xoj_assert(startParam > endParam && startParam.index < this->nbSegments());

    // Drop first and/or last segments if they are too tiny
    SplineSegment firstSegment = getSegment(startParam.index).subdivide(startParam.t).second;
    bool dropFirst = firstSegment.isNegligible();

    SplineSegment lastSegment = getSegment(endParam.index).subdivide(endParam.t).first;
    bool dropLast = lastSegment.isNegligible();

    std::unique_ptr<Spline> clone = std::make_unique<Spline>(
            this->getPoint(startParam),
            this->nbSegments() - startParam.index + endParam.index + 1 - (dropLast ? 1 : 0) - (dropFirst ? 1 : 0));

    if (!dropFirst) {
        clone->addCubicSegment(firstSegment);
    }

    // firstControlPoint of getSegment(startParam.index + 1)
    auto startIt = this->data.cbegin() + SEGMENT_SIZE * (std::ptrdiff_t)startParam.index + SEGMENT_SIZE + 1;
    xoj_assert(startIt <= this->data.cend());
    std::copy(startIt, this->data.cend(), std::back_inserter(clone->data));

    // firstControlPoint of getSegment(endParam.index)
    auto endIt = this->data.cbegin() + SEGMENT_SIZE * (std::ptrdiff_t)endParam.index + 1;
    std::copy(this->data.cbegin() + 1, endIt, std::back_inserter(clone->data));

    if (!dropLast) {
        clone->addCubicSegment(lastSegment);
    }

    xoj_assert(clone->data.size() % SEGMENT_SIZE == 1);
    xoj_assert(clone->nbSegments() ==
               this->nbSegments() - startParam.index + endParam.index + 1 - (dropLast ? 1 : 0) - (dropFirst ? 1 : 0));
    return clone;
}

auto Spline::getThickBoundingBox(double fallbackWidth) const -> Range {
    xoj_assert(!data.empty());
    if (data.front().z == Point::NO_PRESSURE) {
        Range res = getThinBoundingBox();
        res.addPadding(0.5 * fallbackWidth);
        return res;
    }
    Range res;
    for (auto& segment: this->segments()) {
        res = res.unite(segment.getThickBoundingBox());
    }
    return res;
}

auto Spline::getThinBoundingBox() const -> Range {
    Range result;
    for (auto& segment: this->segments()) {
        result = result.unite(segment.getThinBoundingBox());
    }
    return result;
}

Range Spline::getSubSectionBoundingBox(const Path::SubSection& section, const double fallbackWidth) const {
    const bool hasPressure = this->data.front().z != Point::NO_PRESSURE;

    if (section.min.index == section.max.index) {
        SplineSegment seg = this->getSegment(section.min.index).getSubsegment(section.min.t, section.max.t);
        if (hasPressure) {
            return seg.getThickBoundingBox();
        } else {
            auto bbox = seg.getThinBoundingBox();
            bbox.addPadding(0.5 * fallbackWidth);
            return bbox;
        }
    }

    auto getBBox = hasPressure ? &SplineSegment::getThickBoundingBox : &SplineSegment::getThinBoundingBox;

    auto segments = this->segments();
    auto it = segments.iteratorAt(section.min.index);
    SplineSegment firstSegment = it->subdivide(section.min.t).second;
    Range result = (firstSegment.*getBBox)();
    ++it;
    for (auto endIt = segments.iteratorAt(section.max.index); it != endIt; ++it) {
        result = result.unite((*it.*getBBox)());
    }
    SplineSegment lastSegment = it->subdivide(section.max.t).first;
    result = result.unite((lastSegment.*getBBox)());

    if (!hasPressure) {
        result.addPadding(0.5 * fallbackWidth);
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

auto Spline::nbSegments() const -> size_t { return data.empty() ? 0 : (data.size() - 1) / SEGMENT_SIZE; }

void Spline::resize(size_t n) {
    n *= SEGMENT_SIZE;
    ++n;  // corresponding number of points
    if (n < data.size()) {
        data.resize(n);
    }
}

auto Spline::intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex, size_t lastIndex) const
        -> IntersectionParametersContainer {
    return this->intersectWithPaddedBoxTemplate<SplineSegment>(box, firstIndex, lastIndex, this->segments());
}

bool Spline::isInSelection(ShapeContainer* container) {
    if (this->data.empty()) {
        return false;
    }
    if (const Point& p = this->data.front(); !container->contains(p.x, p.y)) {
        return false;
    }
    for (auto&& seg: segments()) {
        if (!seg.isTailInSelection(container, false)) {
            return false;
        }
    }
    return true;
}

double Spline::squaredDistanceToPoint(const Point& p, double veryClose, double toFar) {
    if (this->data.empty()) {
        return toFar;
    }
    double min = toFar;
    for (auto&& seg: this->segments()) {
        // Coarse test to save time
        if (seg.squaredDistanceToHull(p) < min) {
            double d = seg.closestPointTo(p).second;
            if (d < min) {
                if (d < veryClose) {
                    return veryClose;
                }
                min = d;
            }
        }
    }
    return min;
}

void Spline::addToCairo(cairo_t* cr) const {
    if (!this->empty()) {
        const Point& p = this->getFirstKnot();
        cairo_move_to(cr, p.x, p.y);
        for (auto&& seg: this->segments()) {
            cairo_curve_to(cr, seg.firstControlPoint.x, seg.firstControlPoint.y, seg.secondControlPoint.x,
                           seg.secondControlPoint.y, seg.secondKnot.x, seg.secondKnot.y);
        }
    }
}

void Spline::addSectionToCairo(cairo_t* cr, const Path::SubSection& section) const {
    auto segments = this->segments();
    auto it = segments.iteratorAt(section.min.index);

    if (section.max.index == section.min.index) {
        it->getSubsegment(section.min.t, section.max.t).toCairo(cr);
    } else {
        SplineSegment firstSegment = it->subdivide(section.min.t).second;

        firstSegment.toCairo(cr);
        ++it;
        auto endIt = segments.iteratorAt(section.max.index);
        for (; it != endIt; ++it) {
            cairo_curve_to(cr, it->firstControlPoint.x, it->firstControlPoint.y, it->secondControlPoint.x,
                           it->secondControlPoint.y, it->secondKnot.x, it->secondKnot.y);
        }

        SplineSegment seg = it->subdivide(section.max.t).first;
        cairo_curve_to(cr, seg.firstControlPoint.x, seg.firstControlPoint.y, seg.secondControlPoint.x,
                       seg.secondControlPoint.y, seg.secondKnot.x, seg.secondKnot.y);
    }
}

void Spline::addCircularSectionToCairo(cairo_t* cr, const Path::Parameter& startParam,
                                       const Path::Parameter& endParam) const {
    auto segments = this->segments();
    auto it = segments.iteratorAt(startParam.index);
    SplineSegment firstSegment = it->subdivide(startParam.t).second;
    firstSegment.toCairo(cr);

    ++it;
    for (auto endIt = segments.end(); it != endIt; ++it) {
        cairo_curve_to(cr, it->firstControlPoint.x, it->firstControlPoint.y, it->secondControlPoint.x,
                       it->secondControlPoint.y, it->secondKnot.x, it->secondKnot.y);
    }
    it = segments.begin();
    for (auto endIt = segments.iteratorAt(endParam.index); it != endIt; ++it) {
        cairo_curve_to(cr, it->firstControlPoint.x, it->firstControlPoint.y, it->secondControlPoint.x,
                       it->secondControlPoint.y, it->secondKnot.x, it->secondKnot.y);
    }

    SplineSegment seg = it->subdivide(endParam.t).first;
    cairo_curve_to(cr, seg.firstControlPoint.x, seg.firstControlPoint.y, seg.secondControlPoint.x,
                   seg.secondControlPoint.y, seg.secondKnot.x, seg.secondKnot.y);
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
