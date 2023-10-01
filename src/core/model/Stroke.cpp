#include "Stroke.h"

#include <algorithm>  // for min, max, copy
#include <cmath>      // for abs, hypot, sqrt
#include <cstdint>    // for uint64_t
#include <iterator>   // for back_insert_iterator
#include <limits>     // for numeric_limits
#include <numeric>    // for accumulate
#include <optional>   // for optional, nullopt
#include <string>     // for to_string, operator<<

#include <cairo.h>  // for cairo_matrix_translate
#include <glib.h>   // for g_free, g_message

#include "control/tools/splineapproximation/SplineApproximatorSchneider.h"
#include "eraser/PaddedBox.h"                     // for PaddedBox
#include "model/AudioElement.h"                   // for AudioElement
#include "model/Element.h"                        // for Element, ELEMENT_ST...
#include "model/LineStyle.h"                      // for LineStyle
#include "model/Point.h"                          // for Point, Point::NO_PR...
#include "model/path/Path.h"                      // for Path
#include "model/path/PiecewiseLinearPath.h"       // for PiecewiseLinearPath
#include "model/path/Spline.h"                    // for Spline
#include "util/Assert.h"                          // for xoj_assert
#include "util/BasePointerIterator.h"             // for BasePointerIterator
#include "util/Interval.h"                        // for Interval
#include "util/PairView.h"                        // for PairView<>::BaseIte...
#include "util/PlaceholderString.h"               // for PlaceholderString
#include "util/Range.h"                           // for Range
#include "util/Rectangle.h"                       // for Rectangle
#include "util/SmallVector.h"                     // for SmallVector
#include "util/TinyVector.h"                      // for TinyVector
#include "util/i18n.h"                            // for FC, FORMAT_STR
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

#include "config-debug.h"  // for ENABLE_ERASER_DEBUG

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
    pressureSensitive = other->pressureSensitive;

    cloneAudioData(other);
}

auto Stroke::cloneStroke() const -> Stroke* {
    auto* s = new Stroke();
    s->applyStyleFrom(this);
    s->x = this->x;
    s->y = this->y;
    s->Element::width = this->Element::width;
    s->Element::height = this->Element::height;
    s->snappedBounds = this->snappedBounds;
    s->sizeCalculated = this->sizeCalculated;
    s->path = this->path->clone();
    return s;
}

auto Stroke::clone() const -> Element* { return this->cloneStroke(); }

void Stroke::serialize(ObjectOutputStream& out) const {
    out.writeObject("Stroke");

    this->AudioElement::serialize(out);

    out.writeDouble(this->width);

    out.writeInt(this->toolType);

    out.writeInt(fill);

    out.writeInt(this->capStyle);

    if (this->path) {
        this->path->serialize(out);
    }

    this->lineStyle.serialize(out);

    out.endObject();
}

void Stroke::readSerialized(ObjectInputStream& in) {
    in.readObject("Stroke");

    this->AudioElement::readSerialized(in);

    this->width = in.readDouble();

    this->toolType = static_cast<StrokeTool::Value>(in.readInt());

    this->fill = in.readInt();

    this->capStyle = static_cast<StrokeCapStyle>(in.readInt());

    std::string name = in.getNextObjectName();
    if (name == "Spline") {
        path = std::make_shared<Spline>(in);
    } else if (name == "PiecewiseLinearPath") {
        path = std::make_shared<PiecewiseLinearPath>(in);
    } else {
        path = nullptr;
    }

    this->lineStyle.readSerialized(in);

    in.endObject();

    this->pressureSensitive = this->toolType.isPressureSensitive() && this->path && !this->path->empty() &&
                              this->path->getFirstKnot().z != Point::NO_PRESSURE;
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
    return this->path && (this->Element::isInSelection(container) || this->path->isInSelection(container));
}


void Stroke::freeUnusedPointItems() {
    if (path) {
        this->path->freeUnusedPointItems();
    }
}

void Stroke::setToolType(StrokeTool type) { this->toolType = type; }

auto Stroke::getToolType() const -> StrokeTool { return this->toolType; }

void Stroke::setLineStyle(const LineStyle& style) { this->lineStyle = style; }

auto Stroke::getLineStyle() const -> const LineStyle& { return this->lineStyle; }

void Stroke::move(double dx, double dy) {
    path->move(dx, dy);

    for (auto&& p: pointCache) {
        p.x += dx;
        p.y += dy;
    }
    if (this->sizeCalculated) {
        Element::x += dx;
        Element::y += dy;
        Element::snappedBounds.x += dx;
        Element::snappedBounds.y += dy;
    }
}

void Stroke::rotate(double x0, double y0, double th) {
    cairo_matrix_t mat = this->path->rotate(x0, y0, th);

    for (auto&& p: pointCache) {
        cairo_matrix_transform_point(&mat, &p.x, &p.y);
    }
    this->sizeCalculated = false;
}

void Stroke::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    auto [matrix, widthFactor] = this->path->scale(x0, y0, fx, fy, rotation, restoreLineWidth);

    if (restoreLineWidth) {
        for (auto&& p: pointCache) {
            cairo_matrix_transform_point(&matrix, &p.x, &p.y);
        }
    } else {
        for (auto&& p: pointCache) {
            cairo_matrix_transform_point(&matrix, &p.x, &p.y);
            p.z *= widthFactor;
        }
        this->width *= widthFactor;
    }

    this->sizeCalculated = false;
}

auto Stroke::hasPressure() const -> bool { return pressureSensitive; }
void Stroke::setPressureSensitive(bool b) { pressureSensitive = b; }

std::vector<double> Stroke::getPressureValues() const {
    if (this->hasPressure()) {
        return this->path->getPressureValues();
    }
    return {};
}

void Stroke::setPressureValues(const std::vector<double>& pressures) {
    if (this->hasPressure()) {
        this->path->setPressureValues(pressures);

        if (this->path->getType() == Path::SPLINE) {
            this->clearPointCache();
        }
    }
}

auto Stroke::isPointNearby(double x, double y, double veryClose, double toFar) const -> double {
    return this->path->squaredDistanceToPoint(Point(x, y), veryClose, toFar);
}

auto Stroke::intersectWithPaddedBox(const PaddedBox& box) const -> Path::IntersectionParametersContainer {
    if (!this->path) {
        g_warning("Intersecting pathless stroke with padded box. This should never happen!");
        return {};
    }
    return this->path->intersectWithPaddedBox(box);
}

/**
 * Updates the size
 * The size is needed to only redraw the requested part instead of redrawing
 * the whole page (performance reason).
 * Also used for Selected Bounding box.
 */
void Stroke::calcSize() const {
    if (this->hasPressure()) {
        const std::vector<Point>& pts = getPointsToDraw();

        if (pts.empty()) {
            Element::x = 0.0;
            Element::y = 0.0;
            Element::width = 0.0;
            Element::height = 0.0;

            Element::snappedBounds = {0.0, 0.0, 0.0, 0.0};
            return;
        }

        double minSnapX = std::numeric_limits<double>::max();
        double maxSnapX = std::numeric_limits<double>::min();
        double minSnapY = std::numeric_limits<double>::max();
        double maxSnapY = std::numeric_limits<double>::min();

        auto halfThick = 0.0;

        // #pragma omp parralel
        for (auto&& p: pts) {
            halfThick = std::max(halfThick, p.z);
            minSnapX = std::min(minSnapX, p.x);
            minSnapY = std::min(minSnapY, p.y);
            maxSnapX = std::max(maxSnapX, p.x);
            maxSnapY = std::max(maxSnapY, p.y);
        }

        halfThick /= 2.0;

        auto minX = minSnapX - halfThick;
        auto minY = minSnapY - halfThick;
        auto maxX = maxSnapX + halfThick;
        auto maxY = maxSnapY + halfThick;

        Element::x = minX;
        Element::y = minY;
        Element::width = maxX - minX;
        Element::height = maxY - minY;
        Element::snappedBounds = Rectangle<double>(minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY);
    } else {
        auto thinBox = this->path->getThinBoundingBox();
        Element::snappedBounds = {thinBox.minX, thinBox.minY, thinBox.getWidth(), thinBox.getHeight()};
        double halfWidth = 0.5 * this->width;
        Element::x = Element::snappedBounds.x - halfWidth;
        Element::y = Element::snappedBounds.y - halfWidth;
        Element::width = Element::snappedBounds.width + this->width;
        Element::height = Element::snappedBounds.height + this->width;
    }
}

auto Stroke::getErasable() const -> ErasableStroke* { return this->erasable; }

void Stroke::setErasable(ErasableStroke* erasable) { this->erasable = erasable; }

auto Stroke::getStrokeCapStyle() const -> StrokeCapStyle { return this->capStyle; }

void Stroke::setStrokeCapStyle(const StrokeCapStyle capStyle) { this->capStyle = capStyle; }

void Stroke::debugPrint() const {
    g_message("%s", FC(FORMAT_STR("Stroke {1} / hasPressure() = {2}") % (uint64_t)this % this->hasPressure()));

    const std::vector<Point>& data = this->path->getData();
    for (auto&& p: data) {
        g_message("%lf / %lf / %lf", p.x, p.y, p.z);
    }

    g_message("\n");
}

void Stroke::splineFromPLPath() {
    if (!this->path || this->path->empty() || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Stroke::splineFromPLPath called on stroke without a piecewise linear path");
        return;
    }

    if (this->path->nbSegments() == 1) {
        // Only 1 segment. No spline required
        return;
    }

    if (this->hasPressure() && this->path->getLastKnot().z == Point::NO_PRESSURE) {
        /**
         * Backward compatiblity
         * Set an extrapolated pressure value for the last point
         */
        std::shared_ptr<PiecewiseLinearPath> PLPath = std::dynamic_pointer_cast<PiecewiseLinearPath>(this->path);
        PLPath->extrapolateLastPressureValue();
    }

    SplineApproximator::Schneider approximator(this->path->getData());
    approximator.run();
    approximator.printStats();
    this->path = std::make_shared<Spline>(approximator.getSpline());
}

const std::vector<Point>& Stroke::getPointsToDraw() const {
    if (!this->path) {
        g_warning("Stroke::getPointsToDraw call on empty stroke. This should never happen.");
        return pointCache;
    }
    if (this->path->getType() == Path::SPLINE) {
        if (!this->hasPressure()) {
            g_warning("Stroke::getPointsToDraw call on pressureless spline. This should never happen.");
        } else if (this->pointCache.empty()) {
            // Fill the cache now
            dynamic_cast<Spline*>(path.get())->toPoints(pointCache);
        }
        return pointCache;
    }
    if (this->path->getType() == Path::PIECEWISE_LINEAR) {
        return this->path->getData();
    }
    g_warning("Stroke::getPointsToDraw call on unknown path type. This should never happen.");
    return pointCache;
}

void Stroke::resizePointCache(size_t n) { this->pointCache.resize(n); }

void Stroke::clearPointCache() { this->pointCache.clear(); }

void Stroke::addToPointCache(const SplineSegment& seg) {
    seg.toPoints(pointCache);
    pointCache.emplace_back(seg.secondKnot);
}

size_t Stroke::getCacheSize() const { return this->pointCache.size(); }

const Path& Stroke::getPath() const { return *(path.get()); }

bool Stroke::hasPath() const { return path != nullptr; }

auto Stroke::getPathPointer() const -> std::shared_ptr<const Path> { return path; }

void Stroke::setPath(std::shared_ptr<Path> p, const Range* const snappingBox) {
    path = p;

    if (!snappingBox || !this->path || this->path->getFirstKnot().z != Point::NO_PRESSURE) {
        // We cannot deduce the bounding box from the snapping box if the stroke has pressure values
        this->sizeCalculated = false;
    } else {
        xoj_assert(snappingBox->isValid());
        this->snappedBounds = xoj::util::Rectangle<double>(*snappingBox);
        Element::x = snappingBox->minX - 0.5 * this->width;
        Element::y = snappingBox->minY - 0.5 * this->width;
        Element::width = snappingBox->getWidth() + this->width;
        Element::height = snappingBox->getHeight() + this->width;
        this->sizeCalculated = true;
    }
}

void Stroke::unsetSizeCalculated() { this->sizeCalculated = false; }

// TODO: Change the StrokeHandler's behaviour on pressure
// TODO: Adapt SplineHandler
// TODO: Stroke::rotate Stroke::scale Stroke::calcSize Stroke::isInSelection
// TODO: Stroke::scalePressure, hasPressure
// TODO: Stroke::intersects
