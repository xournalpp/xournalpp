/**
 * Xournal++
 *
 * A piecewise linear (PL) path
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#include "PiecewiseLinearPath.h"

#include <cmath>

#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"

#include "Element.h"
#include "Interval.h"
#include "LoopUtil.h"

#ifdef EXTRA_CAREFUL
#include <iomanip>
#include <iostream>
#include <sstream>
#endif

PiecewiseLinearPath& PiecewiseLinearPath::operator=(const std::vector<Point>& vector) {
    data = vector;
    return *this;
}

PiecewiseLinearPath::PiecewiseLinearPath(const Point& firstPoint) { data.push_back(firstPoint); }

PiecewiseLinearPath::PiecewiseLinearPath(const Point& firstPoint, size_t size) {
    data.reserve(size + 1);
    data.push_back(firstPoint);
}

PiecewiseLinearPath::PiecewiseLinearPath(const Point& firstPoint, const Point& secondPoint) {
    data = {firstPoint, secondPoint};
}

PiecewiseLinearPath::PiecewiseLinearPath(ObjectInputStream& in) {
    in.readObject("PiecewiseLinearPath");

    Point* p{};
    int count{};
    in.readData(reinterpret_cast<void**>(&p), &count);
    this->data = std::vector<Point>{p, p + count};
    g_free(p);

    in.endObject();
}

void PiecewiseLinearPath::serialize(ObjectOutputStream& out) const {
    out.writeObject("PiecewiseLinearPath");
    out.writeData(this->data.data(), this->data.size(), sizeof(Point));
    out.endObject();
}

void PiecewiseLinearPath::setFirstPoint(const Point& p) {
    if (data.empty()) {
        data.push_back(p);
    } else {
        data.front() = p;
    }
}

void PiecewiseLinearPath::addLineSegmentTo(const Point& q) { data.push_back(q); }

const LineSegment& PiecewiseLinearPath::getSegment(size_t index) const {
    return *(reinterpret_cast<const LineSegment*>(this->data.data() + index));
}

auto PiecewiseLinearPath::getPoint(const Path::Parameter& parameter) const -> Point {
    if (data.empty()) {
        g_warning("PiecewiseLinearPath::getPoint: Empty path");
        return Point(NAN, NAN);
    }

    size_t index = parameter.index;
    double t = parameter.t;
    size_t n = nbSegments();

    if (index == n - 1 && t == 1.0) {
        return data.back();
    }

    if (index >= n || t < 0.0 || t >= 1.0) {
        g_warning("PiecewiseLinearPath::getPoint: Parameter out of range: %zu >= %zu || %f < 0.0 || %f >= 1.0", index,
                  n, t, t);
        return Point(NAN, NAN);
    }

    if (t == 0.0) {
        return data[index];
    }

    const Point& p = data[index];
    const Point& q = data[index + 1];
    double u = 1 - t;
    return Point(u * p.x + t * q.x, u * p.y + t * q.y, u * p.z + t * q.z);
}

auto PiecewiseLinearPath::clone() const -> std::unique_ptr<Path> {
    std::unique_ptr<Path> clone = std::make_unique<PiecewiseLinearPath>(*this);
    //     std::copy(data.cbegin(), data.cend(), std::back_inserter(clone->data));
    return clone;
}

auto PiecewiseLinearPath::cloneSection(const Path::Parameter& lowerBound, const Path::Parameter& upperBound) const
        -> std::unique_ptr<Path> {

    if (upperBound.index == lowerBound.index) {
        const Point& p = getPoint(lowerBound);
        const Point& q = getPoint(upperBound);
        return std::make_unique<PiecewiseLinearPath>(p, q);
    } else {
        // Create and reserve memory for the clone
        std::unique_ptr<PiecewiseLinearPath> clone =
                std::make_unique<PiecewiseLinearPath>(getPoint(lowerBound), upperBound.index - lowerBound.index + 1);

        // second point of segment of index lowerBound.index
        auto it = data.cbegin() + (std::ptrdiff_t)lowerBound.index + 1;

        // second point of segment of index upperBound.index
        auto endIt = data.cbegin() + (std::ptrdiff_t)upperBound.index + 1;

        std::copy(it, endIt, std::back_inserter(clone->data));

        clone->addLineSegmentTo(getPoint(upperBound));

#ifdef EXTRA_CAREFUL
        if (clone->data.size() != upperBound.index - lowerBound.index + 2) {
            g_warning("PiecewiseLinearPath::cloneSection: wrong clone size: %zu. Indices: %zu and %zu",
                      clone->data.size(), lowerBound.index, upperBound.index);
        }
#endif
        return clone;
    }
}

Rectangle<double> PiecewiseLinearPath::getThinBoundingBox() const {
    if (data.empty()) {
        return {0.0, 0.0, 0.0, 0.0};
    }

    double minSnapX = DBL_MAX;
    double maxSnapX = DBL_MIN;
    double minSnapY = DBL_MAX;
    double maxSnapY = DBL_MIN;

    for (auto&& p: data) {
        minSnapX = std::min(minSnapX, p.x);
        minSnapY = std::min(minSnapY, p.y);

        maxSnapX = std::max(maxSnapX, p.x);
        maxSnapY = std::max(maxSnapY, p.y);
    }

    return {minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY};
}

auto PiecewiseLinearPath::getThickBoundingBox() const -> Rectangle<double> {
    if (data.empty()) {
        return {0.0, 0.0, 0.0, 0.0};
    }

    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;

    double halfThick;

    for (auto&& p: data) {
        halfThick = 0.5 * p.z;

        minX = std::min(minX, p.x - halfThick);
        minY = std::min(minY, p.y - halfThick);

        maxX = std::max(maxX, p.x + halfThick);
        maxY = std::max(maxY, p.y + halfThick);
    }

    return {minX, minY, maxX - minX, maxY - minY};
}

auto PiecewiseLinearPath::getBoundingBoxes() const -> std::pair<Rectangle<double>, Rectangle<double>> {
    if (data.empty()) {
        return {{0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0}};
    }

    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;

    double minSnapX = DBL_MAX;
    double maxSnapX = DBL_MIN;
    double minSnapY = DBL_MAX;
    double maxSnapY = DBL_MIN;

    double halfThick;

    for (auto&& p: data) {
        halfThick = 0.5 * p.z;

        minX = std::min(minX, p.x - halfThick);
        minY = std::min(minY, p.y - halfThick);

        maxX = std::max(maxX, p.x + halfThick);
        maxY = std::max(maxY, p.y + halfThick);

        minSnapX = std::min(minSnapX, p.x);
        minSnapY = std::min(minSnapY, p.y);

        maxSnapX = std::max(maxSnapX, p.x);
        maxSnapY = std::max(maxSnapY, p.y);
    }

    return {{minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY}, {minX, minY, maxX - minX, maxY - minY}};
}

void PiecewiseLinearPath::scalePressure(double factor) {
    for (auto&& p: data) {
        p.z *= factor;
    }
}

void PiecewiseLinearPath::setPressure(const vector<double>& pressure) {
    if (this->nbSegments() != pressure.size()) {
        g_warning("invalid pressure point count: %s, expected %s", std::to_string(pressure.size()).data(),
                  std::to_string(this->nbSegments()).data());
    }

    auto max_size = std::min(pressure.size(), this->nbSegments());
    for (size_t i = 0U; i != max_size; ++i) {
        this->data[i].z = pressure[i];
    }
}

void PiecewiseLinearPath::setSecondToLastPressure(double pressure) {
    if (size_t size = this->data.size(); size >= 2) {
        this->data[size - 2].z = pressure;
    }
}


void PiecewiseLinearPath::clearPressure() {
    for (auto&& p: data) {
        p.z = Point::NO_PRESSURE;
    }
}

void PiecewiseLinearPath::extrapolateLastPressureValue() {
    size_t size = this->nbSegments();
    if (size == 0) {  // No segment (maybe a first point)
        return;
    }
    if (size == 1) {  // Only 1 segment
        this->data.back().z = this->data.front().z;
    } else {
        // Linearly extrapolate the pressure using the second and third to last points
        // Remember that size is the number of segments (= nb of points - 1)
        this->data.back().z = std::max(2 * data[size - 1].z - data[size - 2].z, 0.05);
    }
}

auto PiecewiseLinearPath::nbSegments() const -> size_t { return data.empty() ? 0 : data.size() - 1; }

void PiecewiseLinearPath::resize(size_t n) {
    ++n;  // corresponding number of points
    if (n < data.size()) {
        data.resize(n);
    }
}

auto PiecewiseLinearPath::intersectWithRectangle(const Rectangle<double>& rectangle) const
        -> std::vector<Path::Parameter> {
    if (data.empty()) {
        return {};
    }

    bool startInside = data.front().isInside(rectangle);
    if (data.size() == 1 || (data.size() == 2 && data[0].equalsPos(data[1]))) {
        if (startInside) {
            return {{0, 0.0}, {0, 1.0}};
        } else {
            return {};
        }
    }

    std::vector<Parameter> result;
    if (startInside) {
        result.emplace_back(0, 0.0);
    }

    size_t index = 0;
    auto it1 = data.cbegin();
    auto it2 = std::next(it1);
    auto endIt = data.cend();
    for (auto it1 = data.cbegin(), it2 = std::next(it1); it2 != endIt; it1 = it2++, index++) {
        std::vector<double> intersections = intersectSegmentWithRectangle(*it1, *it2, rectangle);
        std::transform(intersections.cbegin(), intersections.cend(), std::back_inserter(result),
                       [&index](double t) { return Parameter(index, t); });
    }
    if (data.back().isInside(rectangle)) {
        size_t n = data.size() >= 2 ? data.size() - 2 : 0;
        result.emplace_back(n, 1.0);
    }
    /**
     * Do we need to take care of the very improbable cases where the first or last knot lie on the boundary?
     * Could some snapping make those cases actually possible?
     */

#ifdef EXTRA_CAREFUL
    if (result.size() % 2) {
        g_warning("PiecewiseLinearPath::intersectWithRectangle: Odd number. This should never happen");
    }
#endif

    return result;
}

auto PiecewiseLinearPath::intersectWithRectangle(const Rectangle<double>& rectangle, size_t firstIndex,
                                                 size_t lastIndex) const -> std::vector<Path::Parameter> {

#ifdef EXTRA_CAREFUL
    std::stringstream ss;
    ss << "Call: ** rectangle (" << rectangle.x << " ; " << rectangle.y << ") -- (" << rectangle.x + rectangle.width
       << " ; " << rectangle.y + rectangle.height << ")\n";
    ss << "      ** param: (" << firstIndex << " ; " << lastIndex << ")\n";
#endif

    std::vector<Parameter> result;
    auto inserter = std::back_inserter(result);
    size_t index = firstIndex;
    auto it1 = data.cbegin() + (std::ptrdiff_t)index;
    auto it2 = std::next(it1);

    /**
     * The first segment
     */
    std::vector<double> intersections = intersectSegmentWithRectangle(*it1, *it2, rectangle);

#ifdef EXTRA_CAREFUL
    ss << "I ** " << index << " : ";
    for (auto&& t: intersections) {
        ss << t << " ; ";
    }
#endif

    if (it1->isInside(rectangle)) {
        /**
         * The spline starts in the rectangle. Add a fake intersection parameter
         */
#ifdef EXTRA_CAREFUL
        ss << "start in";
#endif
        result.emplace_back(index, 0.0);
    } else if (isPointOnBoundary(*it1, rectangle)) {
        /**
         * Improbable case: the segment begins on the rectangle's boundary
         */
        Point p;
        if (intersections.empty()) {
            p = *it2;
        } else {
            double t = 0.5 * intersections.front();
            double u = 1 - t;
            p.x = u * it1->x + t * it2->x;
            p.y = u * it1->y + t * it2->y;
        }
        if (p.isInside(rectangle)) {
            /**
             * Exceptional case: The segment begins on the rectangle's boundary and goes inwards.
             * Add an intersection point
             */
            result.emplace_back(index, 0.0);
#ifdef EXTRA_CAREFUL
            ss << "exc1";
            g_message("PiecewiseLinearPath::intersectWithRectangle: Exceptional case 1");
        } else {
            ss << "imp1";
            g_message("PiecewiseLinearPath::intersectWithRectangle: Improbable case 1");
#endif
        }
    }
#ifdef EXTRA_CAREFUL
    ss << "\n";
#endif

    std::transform(intersections.begin(), intersections.end(), inserter,
                   [index](double v) { return Parameter(index, v); });

    auto endIt = data.cbegin() + (std::ptrdiff_t)(lastIndex + 2);
    it1 = it2++;  // We already took care of the first segment
    index++;

    for (; it2 != endIt; it1 = it2++, index++) {
        std::vector<double> intersection = intersectSegmentWithRectangle(*it1, *it2, rectangle);
        std::transform(intersection.cbegin(), intersection.cend(), inserter,
                       [&index](double t) { return Parameter(index, t); });
#ifdef EXTRA_CAREFUL
        ss << "I ** " << index << " : ";
        for (auto&& t: intersection) {
            ss << t << " ; ";
        }
        ss << "\n";
#endif
    }

    if (result.size() % 2) {
        if (it1->isInside(rectangle)) {
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
            g_message("PiecewiseLinearPath::intersectWithRectangle: Improbable case 2. May also be a bug.");
#endif
            result.pop_back();
        }
    }
    return result;
}

bool PiecewiseLinearPath::isInSelection(ShapeContainer* container) {
    for (auto&& p: this->data) {
        double px = p.x;
        double py = p.y;

        if (!container->contains(px, py)) {
            return false;
        }
    }

    return true;
}

double PiecewiseLinearPath::squaredDistanceToPoint(const Point& p, double veryClose, double toFar) {
    if (this->data.empty()) {
        return toFar;
    }
    MathVect2 u(p, this->data.front());
    double min = std::min(toFar, u.squaredNorm());
    for (auto it1 = this->data.cbegin(), it2 = it1 + 1, end = this->data.cend(); it2 != end; it1 = it2++) {
        MathVect2 v(*it1, *it2);
        double t = -MathVect2::scalarProduct(u, v) / v.squaredNorm();
        MathVect2 w(p, *it2);
        if (t > 0.0) {
            double m = (t >= 1.0 ? w.squaredNorm() : (u + t * v).squaredNorm());
            if (m < min) {
                if (m <= veryClose) {
                    return veryClose;
                }
                min = m;
            }
        }
        u = w;
    }
    return min;
}

void PiecewiseLinearPath::addToCairo(cairo_t* cr) const {
    for_first_then_each(
            this->data, [cr](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [cr](auto const& other) { cairo_line_to(cr, other.x, other.y); });
}


std::vector<double> PiecewiseLinearPath::intersectSegmentWithRectangle(const Point& p, const Point& q,
                                                                       const Rectangle<double>& rectangle) {

    // Interval of parameters of the line passing through p and q within the rectangle's boundary
    std::optional<Interval<double>> intersections = std::nullopt;
    if (p.x == q.x) {
        // Vertical segment
        double x = p.x;
        if (p.y == q.y || x <= rectangle.x || x >= rectangle.x + rectangle.width) {
            return {};
        }
        double norm = 1.0 / (q.y - p.y);
        double tUp = (rectangle.y - p.y) * norm;
        double tBottom = tUp + rectangle.height * norm;
        intersections = Interval<double>::getInterval(tUp, tBottom);
    } else if (p.y == q.y) {
        // Horizontal segment
        double y = p.y;
        if (y <= rectangle.y || y >= rectangle.y + rectangle.height) {
            return {};
        }
        double norm = 1.0 / (q.x - p.x);
        double tUp = (rectangle.x - p.x) * norm;
        double tBottom = tUp + rectangle.width * norm;
        intersections = Interval<double>::getInterval(tUp, tBottom);
    } else {
        double norm = 1.0 / (q.y - p.y);
        double tUp = (rectangle.y - p.y) * norm;
        double tBottom = tUp + rectangle.height * norm;
        Interval<double> verticalIntersections = Interval<double>::getInterval(tUp, tBottom);

        norm = 1.0 / (q.x - p.x);
        tUp = (rectangle.x - p.x) * norm;
        tBottom = tUp + rectangle.width * norm;
        Interval<double> horizontalIntersections = Interval<double>::getInterval(tUp, tBottom);

        intersections = verticalIntersections.intersect(horizontalIntersections);
    }
    if (!intersections) {
        return {};
    }
    std::vector<double> result;
    if (intersections->min > 0.0 && intersections->min <= 1.0) {
        result.emplace_back(intersections->min);
    }
    if (intersections->max > 0.0 && intersections->max <= 1.0) {
        result.emplace_back(intersections->max);
    }
    return result;
}
