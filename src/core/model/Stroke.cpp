#include "Stroke.h"

#include <algorithm>  // for min, max, copy
#include <cassert>    // for assert
#include <cfloat>     // for DBL_MAX, DBL_MIN
#include <cinttypes>  // for uint64_t
#include <cmath>      // for abs, hypot, sqrt
#include <iterator>   // for back_insert_iterator
#include <numeric>    // for accumulate
#include <optional>   // for optional, nullopt
#include <string>     // for to_string, operator<<

#include <cairo.h>  // for cairo_matrix_translate
#include <glib.h>   // for g_free, g_message

#include "eraser/PaddedBox.h"                     // for PaddedBox
#include "model/AudioElement.h"                   // for AudioElement
#include "model/Element.h"                        // for Element, ELEMENT_ST...
#include "model/LineStyle.h"                      // for LineStyle
#include "model/Point.h"                          // for Point, Point::NO_PR...
#include "util/BasePointerIterator.h"             // for BasePointerIterator
#include "util/Interval.h"                        // for Interval
#include "util/PairView.h"                        // for PairView<>::BaseIte...
#include "util/PlaceholderString.h"               // for PlaceholderString
#include "util/Rectangle.h"                       // for Rectangle
#include "util/SmallVector.h"                     // for SmallVector
#include "util/TinyVector.h"                      // for TinyVector
#include "util/i18n.h"                            // for FC, FORMAT_STR
#include "util/serdesstream.h"                    // for serdes_stream
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

#include "PathParameter.h"  // for PathParameter
#include "config-debug.h"   // for ENABLE_ERASER_DEBUG

using xoj::util::Rectangle;

#define COMMA ,
// #define ENABLE_ERASER_DEBUG // See config-debug.h.in
#ifdef ENABLE_ERASER_DEBUG
#include <iomanip>  // for operator<<, setw
#include <sstream>  // for operator<<, basic_o...

#define DEBUG_ERASER(f) f
#else
#define DEBUG_ERASER(f)
#endif

template <typename Float>
constexpr void updateBounds(Float& x, Float& y, Float& width, Float& height, Rectangle<Float>& snap, Point const& p,
                            double half_width) {
    {
        Float x2 = x + width;
        Float y2 = y + height;

        x = std::min(x, p.x - half_width);
        y = std::min(y, p.y - half_width);
        x2 = std::max(x2, p.x + half_width);
        y2 = std::max(y2, p.y + half_width);
        width = x2 - x;
        height = y2 - y;
    }
    {
        Float snapx2 = snap.x + snap.width;
        Float snapy2 = snap.y + snap.height;

        snap.x = std::min(snap.x, p.x);
        snap.y = std::min(snap.y, p.y);
        snapx2 = std::max(snapx2, p.x);
        snapy2 = std::max(snapy2, p.y);
        snap.width = snapx2 - snap.x;
        snap.height = snapy2 - snap.y;
    }
}


Stroke::Stroke(): AudioElement(ELEMENT_STROKE) {}

Stroke::~Stroke() = default;

/**
 * Clone style attributes, but not the data (position, pressure etc.)
 */
void Stroke::applyStyleFrom(const Stroke* other) {
    setColor(other->getColor());
    setToolType(other->getToolType());
    setWidth(other->getWidth());
    setFill(other->getFill());
    setStrokeCapStyle(other->getStrokeCapStyle());
    setLineStyle(other->getLineStyle());

    cloneAudioData(other);
}

auto Stroke::cloneStroke() const -> Stroke* {
    auto* s = new Stroke();
    s->applyStyleFrom(this);
    s->points = this->points;
    s->x = this->x;
    s->y = this->y;
    s->Element::width = this->Element::width;
    s->Element::height = this->Element::height;
    s->snappedBounds = this->snappedBounds;
    s->sizeCalculated = this->sizeCalculated;
    return s;
}

auto Stroke::clone() const -> Element* { return this->cloneStroke(); }

std::unique_ptr<Stroke> Stroke::cloneSection(const PathParameter& lowerBound, const PathParameter& upperBound) const {
    assert(lowerBound.isValid() && upperBound.isValid());
    assert(lowerBound <= upperBound);
    assert(upperBound.index < this->points.size() - 1);

    auto s = std::make_unique<Stroke>();
    s->applyStyleFrom(this);

    s->points.reserve(upperBound.index - lowerBound.index + 2);

    s->points.emplace_back(this->getPoint(lowerBound));

    auto beginIt = std::next(this->points.cbegin(), (std::ptrdiff_t)lowerBound.index + 1);
    auto endIt = std::next(this->points.cbegin(), (std::ptrdiff_t)upperBound.index + 1);
    std::copy(beginIt, endIt, std::back_inserter(s->points));

    s->points.emplace_back(this->getPoint(upperBound));

    // Remove unused pressure value
    s->points.back().z = Point::NO_PRESSURE;

    return s;
}

std::unique_ptr<Stroke> Stroke::cloneCircularSectionOfClosedStroke(const PathParameter& startParam,
                                                                   const PathParameter& endParam) const {
    assert(startParam.isValid() && endParam.isValid());
    assert(endParam < startParam);
    assert(startParam.index < this->points.size() - 1);

    auto s = std::make_unique<Stroke>();
    s->applyStyleFrom(this);

    s->points.reserve(this->points.size() - startParam.index + endParam.index + 1);

    s->points.emplace_back(this->getPoint(startParam));

    auto startIt = std::next(this->points.cbegin(), (std::ptrdiff_t)startParam.index + 1);
    // Skip the last point: points.back().equalPos(points.front()) == true and we want this point only once
    assert(startIt != this->points.cend());
    std::copy(startIt, std::prev(this->points.cend()), std::back_inserter(s->points));

    auto endIt = std::next(this->points.cbegin(), (std::ptrdiff_t)endParam.index + 1);
    std::copy(this->points.cbegin(), endIt, std::back_inserter(s->points));

    s->points.emplace_back(this->getPoint(endParam));

    // Remove unused pressure value
    s->points.back().z = Point::NO_PRESSURE;

    return s;
}

void Stroke::serialize(ObjectOutputStream& out) const {
    out.writeObject("Stroke");

    this->AudioElement::serialize(out);

    out.writeDouble(this->width);

    out.writeInt(this->toolType);

    out.writeInt(fill);

    out.writeInt(this->capStyle);

    out.writeData(this->points.data(), this->points.size(), sizeof(Point));

    this->lineStyle.serialize(out);

    out.endObject();
}

void Stroke::readSerialized(ObjectInputStream& in) {
    in.readObject("Stroke");

    this->AudioElement::readSerialized(in);

    this->width = in.readDouble();

    this->toolType = static_cast<StrokeTool>(in.readInt());

    this->fill = in.readInt();

    this->capStyle = static_cast<StrokeCapStyle>(in.readInt());

    Point* p{};
    int count{};
    in.readData(reinterpret_cast<void**>(&p), &count);
    this->points = std::vector<Point>{p, p + count};
    g_free(p);
    this->lineStyle.readSerialized(in);

    in.endObject();
}

/**
 * Option to fill the shape:
 *  -1: The shape is not filled
 * 255: The shape is fully opaque filled
 * ...
 *   1: The shape is nearly fully transparent filled
 */
auto Stroke::getFill() const -> int { return fill; }

/**
 * Option to fill the shape:
 *  -1: The shape is not filled
 * 255: The shape is fully opaque filled
 * ...
 *   1: The shape is nearly fully transparent filled
 */
void Stroke::setFill(int fill) { this->fill = fill; }

void Stroke::setWidth(double width) { this->width = width; }

auto Stroke::getWidth() const -> double { return this->width; }

auto Stroke::rescaleWithMirror() -> bool { return true; }

auto Stroke::isInSelection(ShapeContainer* container) const -> bool {
    for (auto&& p: this->points) {
        double px = p.x;
        double py = p.y;

        if (!container->contains(px, py)) {
            return false;
        }
    }

    return true;
}

void Stroke::setFirstPoint(double x, double y) {
    if (!this->points.empty()) {
        Point& p = this->points.front();
        p.x = x;
        p.y = y;
        this->sizeCalculated = false;
    }
}

void Stroke::setLastPoint(double x, double y) { setLastPoint({x, y}); }

void Stroke::setLastPoint(const Point& p) {
    if (!this->points.empty()) {
        this->points.back() = p;
        this->sizeCalculated = false;
    }
}

void Stroke::addPoint(const Point& p) {
    this->points.emplace_back(p);
    updateBounds(Element::x, Element::y, Element::width, Element::height, Element::snappedBounds, p,
                 hasPressure() ? p.z / 2.0 : this->width / 2.0);
}

auto Stroke::getPointCount() const -> int { return this->points.size(); }

auto Stroke::getPointVector() const -> std::vector<Point> const& { return points; }

void Stroke::deletePointsFrom(int index) {
    points.resize(std::min(size_t(index), points.size()));
    this->sizeCalculated = false;
}

void Stroke::deletePoint(int index) {
    this->points.erase(std::next(begin(this->points), index));
    this->sizeCalculated = false;
}

auto Stroke::getPoint(int index) const -> Point {
    if (index < 0 || index >= this->points.size()) {
        g_warning("Stroke::getPoint(%i) out of bounds!", index);
        return Point(0, 0, Point::NO_PRESSURE);
    }
    return points.at(index);
}

Point Stroke::getPoint(PathParameter parameter) const {
    assert(parameter.isValid() && parameter.index < this->points.size() - 1);

    const Point& p = this->points[parameter.index];
    if (parameter.index == this->points.size() - 2) {
        // Need to handle the pressure value separately, since the last pressure value of a stroke is not set.
        Point q = this->points[parameter.index + 1];
        q.z = p.z;
        return p.relativeLineTo(q, parameter.t);
    }
    const Point& q = this->points[parameter.index + 1];
    return p.relativeLineTo(q, parameter.t);
}

auto Stroke::getPoints() const -> const Point* { return this->points.data(); }

void Stroke::setPointVector(const std::vector<Point>& other, const Range* const snappingBox) {
    this->points = other;
    if (!snappingBox || this->points.empty() || this->points.front().z != Point::NO_PRESSURE) {
        // We cannot deduce the bounding box from the snapping box if the stroke has pressure values
        this->sizeCalculated = false;
    } else {
        assert(snappingBox->isValid());
        this->snappedBounds = xoj::util::Rectangle<double>(*snappingBox);
        Element::x = snappingBox->minX - 0.5 * this->width;
        Element::y = snappingBox->minY - 0.5 * this->width;
        Element::width = snappingBox->getWidth() + this->width;
        Element::height = snappingBox->getHeight() + this->width;
        this->sizeCalculated = true;
    }
}

void Stroke::freeUnusedPointItems() { this->points = {begin(this->points), end(this->points)}; }

void Stroke::setToolType(StrokeTool type) { this->toolType = type; }

auto Stroke::getToolType() const -> StrokeTool { return this->toolType; }

void Stroke::setLineStyle(const LineStyle& style) { this->lineStyle = style; }

auto Stroke::getLineStyle() const -> const LineStyle& { return this->lineStyle; }

void Stroke::move(double dx, double dy) {
    for (auto&& point: points) {
        point.x += dx;
        point.y += dy;
    }
    Element::x += dx;
    Element::y += dy;
    Element::snappedBounds = Element::snappedBounds.translated(dx, dy);
}

void Stroke::rotate(double x0, double y0, double th) {
    cairo_matrix_t rotMatrix;
    cairo_matrix_init_identity(&rotMatrix);
    cairo_matrix_translate(&rotMatrix, x0, y0);
    cairo_matrix_rotate(&rotMatrix, th);
    cairo_matrix_translate(&rotMatrix, -x0, -y0);

    for (auto&& p: points) { cairo_matrix_transform_point(&rotMatrix, &p.x, &p.y); }
    this->sizeCalculated = false;
    // Width and Height will likely be changed after this operation
}

void Stroke::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    double fz = (restoreLineWidth) ? 1 : sqrt(std::abs(fx * fy));
    cairo_matrix_t scaleMatrix;
    cairo_matrix_init_identity(&scaleMatrix);
    cairo_matrix_translate(&scaleMatrix, x0, y0);
    cairo_matrix_rotate(&scaleMatrix, rotation);
    cairo_matrix_scale(&scaleMatrix, fx, fy);
    cairo_matrix_rotate(&scaleMatrix, -rotation);
    cairo_matrix_translate(&scaleMatrix, -x0, -y0);

    for (auto&& p: points) {
        cairo_matrix_transform_point(&scaleMatrix, &p.x, &p.y);

        if (p.z != Point::NO_PRESSURE) {
            p.z *= fz;
        }
    }
    this->width *= fz;

    this->sizeCalculated = false;
}

auto Stroke::hasPressure() const -> bool {
    if (!this->points.empty()) {
        return this->points[0].z != Point::NO_PRESSURE;
    }
    return false;
}

auto Stroke::getAvgPressure() const -> double {
    return std::accumulate(begin(this->points), end(this->points), 0.0,
                           [](double l, Point const& p) { return l + p.z; }) /
           this->points.size();
}

void Stroke::scalePressure(double factor) {
    if (!hasPressure()) {
        return;
    }
    for (auto&& p: this->points) { p.z *= factor; }
}

void Stroke::clearPressure() {
    for (auto&& p: points) { p.z = Point::NO_PRESSURE; }
}

void Stroke::setLastPressure(double pressure) {
    if (!this->points.empty()) {
        this->points.back().z = pressure;
    }
}

void Stroke::setSecondToLastPressure(double pressure) {
    auto const pointCount = this->getPointCount();
    if (pointCount >= 2) {
        this->points[pointCount - 2].z = pressure;
    }
}

void Stroke::setPressure(const std::vector<double>& pressure) {
    // The last pressure is not used - as there is no line drawn from this point
    if (this->points.size() - 1 != pressure.size()) {
        g_warning("invalid pressure point count: %s, expected %s", std::to_string(pressure.size()).data(),
                  std::to_string(this->points.size() - 1).data());
    }

    auto max_size = std::min(pressure.size(), this->points.size() - 1);
    for (size_t i = 0U; i != max_size; ++i) { this->points[i].z = pressure[i]; }
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize) const -> bool {
    return intersects(x, y, halfEraserSize, nullptr);
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize, double* gap) const -> bool {
    if (this->points.empty()) {
        return false;
    }

    double x1 = x - halfEraserSize;
    double x2 = x + halfEraserSize;
    double y1 = y - halfEraserSize;
    double y2 = y + halfEraserSize;

    double lastX = points[0].x;
    double lastY = points[0].y;
    for (auto&& point: points) {
        double px = point.x;
        double py = point.y;

        if (px >= x1 && py >= y1 && px <= x2 && py <= y2) {
            if (gap) {
                *gap = 0;
            }
            return true;
        }

        double len = hypot(px - lastX, py - lastY);
        if (len >= halfEraserSize) {
            /**
             * The distance of the center of the eraser box to the line passing through (lastx, lasty) and (px, py)
             */
            double p = std::abs((x - lastX) * (lastY - py) + (y - lastY) * (px - lastX)) / len;

            // If the distance p of the center of the eraser box to the (full) line is in the range,
            // we check whether the eraser box is not too far from the line segment through the two points.

            if (p <= halfEraserSize) {
                double centerX = (lastX + px) / 2;
                double centerY = (lastY + py) / 2;
                double distance = hypot(x - centerX, y - centerY);

                // For the above check we imagine a circle whose center is the mid point of the two points of the stroke
                // and whose radius is half the length of the line segment plus half the diameter of the eraser box
                // plus some small padding
                // If the center of the eraser box lies within that circle then we consider it to be close enough

                distance -= halfEraserSize * std::sqrt(2);

                constexpr double PADDING = 0.1;

                if (distance <= len / 2 + PADDING) {
                    if (gap) {
                        *gap = distance;
                    }
                    return true;
                }
            }
        }

        lastX = px;
        lastY = py;
    }

    return false;
}


/**
 * @brief Get the interval of length parameters where the line (pq) is in the rectangle.
 * @param p First point
 * @param q Second point
 * @param rectangle The rectangle
 * @return Optional interval res.
 * The line enters the rectangle at the point  res.min * p + (1 - res.min) * q
 * The line leaves the rectangle at the point  res.max * p + (1 - res.max) * q
 */
static std::optional<Interval<double>> intersectLineWithRectangle(const Point& p, const Point& q,
                                                                  const Rectangle<double>& rectangle) {
    auto intersectLineWithStrip = [](double a1, double a2, double stripAMin, double stripWidth) {
        // a1, a2 are coordinates along an axis orthogonal to the strip
        double norm = 1.0 / (a2 - a1);
        double t1 = (stripAMin - a1) * norm;
        double t2 = t1 + stripWidth * norm;
        return Interval<double>::getInterval(t1, t2);
    };

    if (p.x == q.x) {
        if (p.y == q.y) {
            // Single dot
            return std::nullopt;
        }
        // Vertical segment
        if (rectangle.x < p.x && p.x < rectangle.x + rectangle.width) {
            return intersectLineWithStrip(p.y, q.y, rectangle.y, rectangle.height);
        }
        return std::nullopt;
    }

    if (p.y == q.y) {
        // Horizontal segment
        if (rectangle.y < p.y && p.y < rectangle.y + rectangle.height) {
            return intersectLineWithStrip(p.x, q.x, rectangle.x, rectangle.width);
        }
        return std::nullopt;
    }

    // Generic case
    Interval<double> verticalIntersections = intersectLineWithStrip(p.y, q.y, rectangle.y, rectangle.height);
    Interval<double> horizontalIntersections = intersectLineWithStrip(p.x, q.x, rectangle.x, rectangle.width);

    return verticalIntersections.intersect(horizontalIntersections);
}

/**
 * Same as intersectLineWithRectangle but only returns parameters between 0 and 1
 * (corresponding to points between p and q)
 */
static TinyVector<double, 2> intersectLineSegmentWithRectangle(const Point& p, const Point& q,
                                                               const Rectangle<double>& rectangle) {
    std::optional<Interval<double>> intersections = intersectLineWithRectangle(p, q, rectangle);

    if (intersections) {
        TinyVector<double, 2> result;
        if (intersections->min > 0.0 && intersections->min <= 1.0) {
            result.emplace_back(intersections->min);
        }
        if (intersections->max > 0.0 && intersections->max <= 1.0) {
            result.emplace_back(intersections->max);
        }
        return result;
    }

    return {};
}

auto Stroke::intersectWithPaddedBox(const PaddedBox& box) const -> IntersectionParametersContainer {
    auto pointCount = this->points.size();
    if (pointCount < 2) {
        if (pointCount == 1 && this->points.back().isInside(box.getInnerRectangle())) {
            IntersectionParametersContainer result;
            result.emplace_back(0U, 0.0);
            result.emplace_back(0U, 0.0);
            return result;
        }
        return IntersectionParametersContainer();
    }
    return this->intersectWithPaddedBox(box, 0, pointCount - 2);
}

auto Stroke::intersectWithPaddedBox(const PaddedBox& box, size_t firstIndex, size_t lastIndex) const
        -> IntersectionParametersContainer {
    assert(firstIndex <= lastIndex && lastIndex < this->points.size() - 1);

    const auto innerBox = box.getInnerRectangle();
    const auto outerBox = box.getOuterRectangle();

    struct Flags {
        bool isInsideOuter;
        bool wentInsideInner;
        bool lastSegmentEndedOnBoundary;
    };

    auto initializeFlagsFromHalfTangentAtFirstKnot =
            [&outerBox, &innerBox](const Point& firstKnot, const Point& halfTangentControlPoint) -> Flags {
        if (firstKnot.isInside(innerBox)) {
            return {true, true, false};
        } else if (firstKnot.isInside(outerBox)) {
            std::optional<Interval<double>> innerLineIntersections =
                    intersectLineWithRectangle(firstKnot, halfTangentControlPoint, innerBox);
            // If the half tangent goes towards to inner box, say we've been inside the inner box
            return {true, innerLineIntersections && innerLineIntersections.value().max <= 0.0, false};
        }
        return {false, false, false};
    };

    size_t index = firstIndex;

    const PairView segments(this->points);
    auto segmentIt = std::next(segments.begin(), (std::ptrdiff_t)index);

    Flags flags = initializeFlagsFromHalfTangentAtFirstKnot(segmentIt.first(), segmentIt.second());

    DEBUG_ERASER(auto debugstream = serdes_stream<std::stringstream>();
                 debugstream << "Stroke::intersectWithPaddedBox debug:\n"; debugstream << std::boolalpha;
                 debugstream << "| * flags.isInsideOuter              = " << flags.isInsideOuter << std::endl;
                 debugstream << "| * flags.wentInsideInner            = " << flags.wentInsideInner << std::endl;
                 debugstream << "| * flags.lastSegmentEndedOnBoundary = " << flags.lastSegmentEndedOnBoundary
                             << std::endl;
                 debugstream << std::fixed;)

    IntersectionParametersContainer result;
    if (flags.isInsideOuter) {
        // We start inside the padded box. Add a fake intersection parameter
        result.emplace_back(firstIndex, 0.0);
    }

    auto processSegment = [&flags, &outerBox, &innerBox, &result DEBUG_ERASER(COMMA & debugstream)](
                                  const Point& firstKnot, const Point& secondKnot, size_t index) {
        DEBUG_ERASER(debugstream << "| * Segment " << std::setw(3) << index << std::setprecision(5);
                     debugstream << ": (" << std::setw(9) << firstKnot.x;
                     debugstream << " ; " << std::setw(9) << firstKnot.y;
                     debugstream << ") -- (" << std::setw(9) << secondKnot.x;
                     debugstream << " ; " << std::setw(9) << secondKnot.y;
                     debugstream << ")\n"
                                 << std::setprecision(17);  // High precision to detect numerical inaccuracy
        )

        auto outerIntersections = intersectLineSegmentWithRectangle(firstKnot, secondKnot, outerBox);
        if (!outerIntersections.empty() || firstKnot.isInside(outerBox) || secondKnot.isInside(outerBox)) {
            // Some part of the segment lies inside the padded box
            auto innerIntersections = intersectLineSegmentWithRectangle(firstKnot, secondKnot, innerBox);
            auto itInner = innerIntersections.begin();
            auto itInnerEnd = innerIntersections.end();

            auto skipInnerIntersectionsBelowValue = [&itInner, itInnerEnd](double upToValue) -> bool {
                bool skipSome = false;
                while (itInner != itInnerEnd && *itInner < upToValue) {
                    skipSome = true;
                    ++itInner;
                }
                return skipSome;
            };

            if (flags.lastSegmentEndedOnBoundary) {
                flags.lastSegmentEndedOnBoundary = false;
                Point p = secondKnot;
                if (!outerIntersections.empty()) {
                    double t = 0.5 * outerIntersections.front();
                    p = firstKnot.relativeLineTo(secondKnot, t);
                }
                if (p.isInside(outerBox) != (result.size() % 2 != 0)) {
                    // The stroke bounced back on the box border and never got in or out
                    // Remove the last intersection point
                    assert(!result.empty());
                    result.pop_back();
                }
            }

            for (auto outerIntersection: outerIntersections) {
                flags.wentInsideInner |= skipInnerIntersectionsBelowValue(outerIntersection);

                DEBUG_ERASER(debugstream << "|  |  ** "
                                         << "wentInsideInner = " << flags.wentInsideInner << std::endl;)

                if (!flags.isInsideOuter || flags.wentInsideInner) {
                    result.emplace_back(index, outerIntersection);
                    if (outerIntersection == 1.0) {
                        flags.lastSegmentEndedOnBoundary = true;
                    }

                    DEBUG_ERASER(
                            if (!flags.isInsideOuter) {
                                debugstream << "|  |  ** going-in  (" << std::setw(3) << index << "," << std::setw(20)
                                            << outerIntersection << ")" << std::endl;
                            } if (flags.wentInsideInner) {
                                debugstream << "|  |  ** going-out (" << std::setw(3) << index << "," << std::setw(20)
                                            << outerIntersection << ")" << std::endl;
                            })
                } else {
                    assert(!result.empty());
                    DEBUG_ERASER(debugstream << "|  |  ** popping   (" << std::setw(3) << result.back().index << ","
                                             << std::setw(20) << result.back().t << ")" << std::endl;)
                    result.pop_back();
                }
                flags.wentInsideInner = false;
                flags.isInsideOuter = !flags.isInsideOuter;
            }
            if (itInner != itInnerEnd) {
                flags.wentInsideInner = true;
            }
        }
        DEBUG_ERASER(debugstream << "|  |__** result.size() = " << std::setw(3) << result.size() << std::endl;)
    };

    auto endSegmentIt = std::next(segments.begin(), (std::ptrdiff_t)(lastIndex + 1));
    for (; segmentIt != endSegmentIt; segmentIt++, index++) {
        processSegment(segmentIt.first(), segmentIt.second(), index);
    }

    auto isHalfTangentAtLastKnotGoingTowardInnerBox =
            [&innerBox, &outerBox](const Point& lastKnot, const Point& halfTangentControlPoint) -> bool {
        assert(lastKnot.isInside(outerBox));
        std::optional<Interval<double>> innerLineIntersections =
                intersectLineWithRectangle(lastKnot, halfTangentControlPoint, innerBox);
        return innerLineIntersections && innerLineIntersections.value().max < 0.0;
    };

    /**
     * Due to numerical imprecision, we could get inconsistent results
     * (typically when a stroke's point lies on outerBox' boundary).
     * We try and detect those cases, and simply drop them
     */
    bool inconsistentResults = false;
    if (result.size() % 2) {
        // Not necessarily inconsistent: could be the stroke ends in outerBox
        --segmentIt;
        const Point& lastPoint = segmentIt.second();

        DEBUG_ERASER(debugstream << "|  |  Odd number of intersection points" << std::endl;)

        if (lastPoint.isInside(outerBox)) {
            if (flags.wentInsideInner || isHalfTangentAtLastKnotGoingTowardInnerBox(lastPoint, segmentIt.first())) {
                result.emplace_back(index - 1, 1.0);
                DEBUG_ERASER(debugstream << "|  |  ** pushing   (" << std::setw(3) << result.back().index << ","
                                         << std::setw(20) << result.back().t << ")" << std::endl;)
            } else {
                DEBUG_ERASER(debugstream << "|  |  ** popping   (" << std::setw(3) << result.back().index << ","
                                         << std::setw(20) << result.back().t << ")" << std::endl;)
                result.pop_back();
            }
        } else {
            inconsistentResults = true;
        }
    }

    // Returns true if the result is inconsistent
    auto checkSanity = [&outerBox,
                        this DEBUG_ERASER(COMMA & debugstream)](const IntersectionParametersContainer& res) -> bool {
        for (auto it1 = res.begin(), it2 = std::next(it1), end = res.end(); it1 != end; it1 += 2, it2 += 2) {
            const auto& paramStart = *it1;
            const auto& paramLast = *it2;
            DEBUG_ERASER(debugstream << "| SanityCheck on parameters (" << paramStart.index << " ; " << paramStart.t
                                     << ") -- (" << paramLast.index << " ; " << paramLast.t << ")" << std::endl;)
            Point testPoint;
            // Get a point on the stroke within the interval of parameters
            if (paramStart.index == paramLast.index) {
                testPoint = this->getPoint(PathParameter(paramStart.index, 0.5 * (paramStart.t + paramLast.t)));
                DEBUG_ERASER(debugstream << "| -------- getting point (" << paramStart.index << " ; "
                                         << 0.5 * (paramStart.t + paramLast.t) << ")" << std::endl;)
            } else {
                testPoint = this->getPoint((paramStart.index + paramLast.index + 1) / 2);
                DEBUG_ERASER(debugstream << "| -------- getting point (" << (paramStart.index + paramLast.index + 1) / 2
                                         << " ; 0.0)" << std::endl;)
            }
            if (!testPoint.isInside(outerBox)) {
                DEBUG_ERASER(debugstream << "| --------- It is not in !!" << std::endl;)
                return true;
            }
            DEBUG_ERASER(debugstream << "| --------- It is in." << std::endl;)
        }
        return false;
    };

    inconsistentResults = inconsistentResults || checkSanity(result);

    if (inconsistentResults) {
        DEBUG_ERASER(debugstream << "| Inconsistent results!\n"; debugstream << "| * innerBox = (";
                     debugstream << std::setprecision(5); debugstream << std::setw(9) << innerBox.x << " ; ";
                     debugstream << std::setw(9) << innerBox.x + innerBox.width << ") -- (";
                     debugstream << std::setw(9) << innerBox.y << " ; ";
                     debugstream << std::setw(9) << innerBox.y + innerBox.height << ")\n";
                     debugstream << "| * outerBox = ("; debugstream << std::setw(9) << outerBox.x << " ; ";
                     debugstream << std::setw(9) << outerBox.x + outerBox.width << ") -- (";
                     debugstream << std::setw(9) << outerBox.y << " ; ";
                     debugstream << std::setw(9) << outerBox.y + outerBox.height << ")\n";
                     std::cout << debugstream.str();)
        return {};
    }

    return result;
}

/**
 * Updates the size
 * The size is needed to only redraw the requested part instead of redrawing
 * the whole page (performance reason).
 * Also used for Selected Bounding box.
 */
void Stroke::calcSize() const {
    if (this->points.empty()) {
        Element::x = 0;
        Element::y = 0;

        // The size of the rectangle, not the size of the pen!
        Element::width = 0;
        Element::height = 0;

        // used for snapping
        Element::snappedBounds = Rectangle<double>{};
    }

    double minSnapX = DBL_MAX;
    double maxSnapX = DBL_MIN;
    double minSnapY = DBL_MAX;
    double maxSnapY = DBL_MIN;

    auto halfThick = 0.0;

    //#pragma omp parralel
    for (auto&& p: points) {
        halfThick = std::max(halfThick, p.z);
        minSnapX = std::min(minSnapX, p.x);
        minSnapY = std::min(minSnapY, p.y);
        maxSnapX = std::max(maxSnapX, p.x);
        maxSnapY = std::max(maxSnapY, p.y);
    }

    halfThick = points[0].z != Point::NO_PRESSURE ? halfThick / 2.0 : this->width / 2.0;

    auto minX = minSnapX - halfThick;
    auto minY = minSnapY - halfThick;
    auto maxX = maxSnapX + halfThick;
    auto maxY = maxSnapY + halfThick;

    Element::x = minX;
    Element::y = minY;
    Element::width = maxX - minX;
    Element::height = maxY - minY;
    Element::snappedBounds = Rectangle<double>(minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY);
}

auto Stroke::getErasable() const -> ErasableStroke* { return this->erasable; }

void Stroke::setErasable(ErasableStroke* erasable) { this->erasable = erasable; }

auto Stroke::getStrokeCapStyle() const -> StrokeCapStyle { return this->capStyle; }

void Stroke::setStrokeCapStyle(const StrokeCapStyle capStyle) { this->capStyle = capStyle; }

void Stroke::debugPrint() const {
    g_message("%s", FC(FORMAT_STR("Stroke {1} / hasPressure() = {2}") % (uint64_t)this % this->hasPressure()));

    for (auto&& p: points) { g_message("%lf / %lf / %lf", p.x, p.y, p.z); }

    g_message("\n");
}
