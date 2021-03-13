#include "Stroke.h"

#include <cmath>
#include <numeric>

// #include <execinfo.h>

#include "model/Path.h"
#include "serializing/ObjectInputStream.h"
#include "serializing/ObjectOutputStream.h"

#include "i18n.h"

Stroke::Stroke(): AudioElement(ELEMENT_STROKE) {}

Stroke::~Stroke() = default;

/**
 * Clone style attributes, but not the data (position, width etc.)
 */
void Stroke::applyStyleFrom(const Stroke* other) {
    setColor(other->getColor());
    setToolType(other->getToolType());
    setWidth(other->getWidth());
    setFill(other->getFill());
    setLineStyle(other->getLineStyle());

    cloneAudioData(other);
}

auto Stroke::cloneStroke() const -> Stroke* {
    auto* s = new Stroke();
    s->applyStyleFrom(this);
    s->x = this->x;
    s->y = this->y;
    s->width = this->width;  // stroke width, not bounding box width
    s->Element::width = this->Element::width;
    s->Element::height = this->Element::height;
    s->snappedBounds = this->snappedBounds;
    s->sizeCalculated = this->sizeCalculated;
    s->path = this->path->clone();
    s->pressureSensitive = this->pressureSensitive;
    return s;
}

auto Stroke::clone() -> Element* { return this->cloneStroke(); }

void Stroke::serialize(ObjectOutputStream& out) {
    out.writeObject("Stroke");

    serializeAudioElement(out);

    out.writeDouble(this->width);

    out.writeInt(this->toolType);

    out.writeInt(fill);

    this->path->serialize(out);

    this->lineStyle.serialize(out);

    out.endObject();
}

void Stroke::readSerialized(ObjectInputStream& in) {
    in.readObject("Stroke");

    readSerializedAudioElement(in);

    this->width = in.readDouble();

    this->toolType = static_cast<StrokeTool>(in.readInt());

    this->fill = in.readInt();

    string name = in.getNextObjectName();
    if (name == "Spline") {
        path = std::make_shared<Spline>(in);
    } else if (name == "PiecewiseLinearPath") {
        path = std::make_shared<PiecewiseLinearPath>(in);
    } else {
        path = nullptr;
    }

    this->lineStyle.readSerialized(in);

    in.endObject();

    this->pressureSensitive = this->toolType == STROKE_TOOL_PEN && this->path && !this->path->empty() &&
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

auto Stroke::isInSelection(ShapeContainer* container) -> bool {
    return this->path && (this->Element::isInSelection(container) || this->path->isInSelection(container));
}

// void Stroke::setFirstPoint(double x, double y) {
//     if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
//         g_warning("Use of deprecated Stroke::setFirstPoint on stroke without piecewise linear path");
//         return;
//     }
//     if (!this->path->empty()) {
//         std::shared_ptr<PiecewiseLinearPath> PLPath = std::dynamic_pointer_cast<PiecewiseLinearPath>(this->path);
//         Point p(x, y, PLPath->getFirstKnot().z);
//         PLPath->setFirstPoint(p);
//         this->sizeCalculated = false;
//     }
// }

// void Stroke::setLastPoint(double x, double y) { setLastPoint({x, y}); }
//
// void Stroke::setLastPoint(const Point& p) {
//     if (!this->points.empty()) {
//         this->points.back() = p;
//         this->sizeCalculated = false;
//     }
// }

void Stroke::addPoint(const Point& p) {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::addPoint on stroke without piecewise linear path");
        //         void* array[10];
        //         int size = backtrace(array, 10);
        //         backtrace_symbols_fd(array, size, STDERR_FILENO);
        return;
    }
    std::shared_ptr<PiecewiseLinearPath> PLPath = std::dynamic_pointer_cast<PiecewiseLinearPath>(this->path);
    PLPath->addLineSegmentTo(p);
    this->sizeCalculated = false;
}

auto Stroke::getPointCount() const -> int {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::getPointCount on stroke without piecewise linear path");
        //         void* array[10];
        //         int size = backtrace(array, 10);
        //         backtrace_symbols_fd(array, size, STDERR_FILENO);
        return 0;
    }
    return this->path->getData().size();
}

auto Stroke::getPointVector() const -> std::vector<Point> const& {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::getPointVector on stroke without piecewise linear path");
        //         void* array[10];
        //         int size = backtrace(array, 10);
        //         backtrace_symbols_fd(array, size, STDERR_FILENO);
    }
    return this->path->getData();
}

// void Stroke::deletePointsFrom(int index) { points.resize(std::min(size_t(index), points.size())); }

// void Stroke::deletePoint(int index) { this->points.erase(std::next(begin(this->points), index)); }

auto Stroke::getPoint(int index) const -> Point {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::getPoint on stroke without piecewise linear path");
        //         void* array[10];
        //         int size = backtrace(array, 10);
        //         backtrace_symbols_fd(array, size, STDERR_FILENO);
        return Point(0, 0);
    }
    const std::vector<Point>& data = this->path->getData();
    if (index < 0 || index >= data.size()) {
        g_warning("Stroke::getPoint(%i) out of bounds!", index);
        return Point(0, 0, Point::NO_PRESSURE);
    }
    return data.at(index);
}

auto Stroke::getPoints() const -> const Point* {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::getPoint on stroke without piecewise linear path");
        //         void* array[10];
        //         int size = backtrace(array, 10);
        //         backtrace_symbols_fd(array, size, STDERR_FILENO);
    }
    return this->path->getData().data();
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
    std::pair<cairo_matrix_t, double> res = this->path->scale(x0, y0, fx, fy, rotation, restoreLineWidth);

    if (restoreLineWidth) {
        for (auto&& p: pointCache) {
            cairo_matrix_transform_point(&(res.first), &p.x, &p.y);
        }
    } else {
        for (auto&& p: pointCache) {
            cairo_matrix_transform_point(&(res.first), &p.x, &p.y);
            p.z *= res.second;
        }
        this->width *= res.second;
    }

    this->sizeCalculated = false;
}

auto Stroke::hasPressure() const -> bool { return pressureSensitive; }
void Stroke::setPressureSensitive(bool b) { pressureSensitive = b; }

auto Stroke::getAvgPressure() const -> double {
    if (!hasPressure()) {
        return Point::NO_PRESSURE;
    }
    /**
     * This formula is imperfect for splines, but it does not matter much
     */
    const std::vector<Point>& data = this->path->getData();
    return std::accumulate(begin(data), end(data), 0.0, [](double l, Point const& p) { return l + p.z; }) /
           (double)data.size();
}

void Stroke::scalePressure(double factor){};  // this->path->scalePressure(factor); }

// void Stroke::clearPressure() {
//     for (auto&& p: points) {
//         p.z = Point::NO_PRESSURE;
//     }
// }

void Stroke::setSecondToLastPressure(double pressure) {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::getPressure on stroke without piecewise linear path");
        return;
    }
    std::dynamic_pointer_cast<PiecewiseLinearPath>(this->path)->setSecondToLastPressure(pressure);
}

// void Stroke::setLastPressure(double pressure) {
//     if (!this->points.empty()) {
//         this->points.back().z = pressure;
//     }
// }

void Stroke::setPressure(const vector<double>& pressure) {
    if (!this->path || this->path->getType() != Path::PIECEWISE_LINEAR) {
        g_warning("Use of deprecated Stroke::getPressure on stroke without piecewise linear path");
        return;
    }
    std::dynamic_pointer_cast<PiecewiseLinearPath>(this->path)->setPressure(pressure);
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize) -> bool {
    Rectangle<double> eraserBox(x - halfEraserSize, y - halfEraserSize, 2 * halfEraserSize, 2 * halfEraserSize);
    intersectionParameters = path->intersectWithRectangle(eraserBox);
    return !intersectionParameters.empty();
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize, double* gap) -> bool {
    double dMin = halfEraserSize * halfEraserSize;
    double dMax = 4.0 * dMin;
    double dSquared = this->path->squaredDistanceToPoint(Point(x, y), dMin, dMax);
    if (dSquared < dMax) {
        if (gap) {
            *gap = std::max(0.0, std::sqrt(dSquared) - halfEraserSize);
        }
        return true;
    }
    return false;
}

/**
 * Updates the size
 * The size is needed to only redraw the requested part instead of redrawing
 * the whole page (performance reason).
 * Also used for Selected Bounding box.
 */
void Stroke::calcSize() const {
    if (this->path->getType() == Path::PIECEWISE_LINEAR && hasPressure()) {
        std::shared_ptr<PiecewiseLinearPath> ptr = std::dynamic_pointer_cast<PiecewiseLinearPath>(this->path);
        std::pair<Rectangle<double>, Rectangle<double>> boxes = ptr->getBoundingBoxes();

        Element::x = boxes.second.x;
        Element::y = boxes.second.y;
        Element::width = boxes.second.width;
        Element::height = boxes.second.height;

        Element::snappedBounds = boxes.first;
    } else {
        Element::snappedBounds = this->path->getThinBoundingBox();
        double halfWidth = 0.5 * this->width;
        Element::x = Element::snappedBounds.x - halfWidth;
        Element::y = Element::snappedBounds.y - halfWidth;
        Element::width = Element::snappedBounds.width + width;
        Element::height = Element::snappedBounds.height + width;
    }
}

auto Stroke::getEraseable() -> EraseableStroke* { return this->eraseable; }

void Stroke::setEraseable(EraseableStroke* eraseable) { this->eraseable = eraseable; }

void Stroke::debugPrint() {
    g_message("%s", FC(FORMAT_STR("Stroke {1} / hasPressure() = {2}") % (uint64_t)this % this->hasPressure()));

    const std::vector<Point>& data = this->path->getData();
    for (auto&& p: data) {
        g_message("%lf / %lf", p.x, p.y);
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

    this->path = std::make_shared<Spline>(Spline::getSchneiderApproximation(this->path->getData()));
}

const std::vector<Point>& Stroke::getPointsToDraw() {
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

void Stroke::setPath(std::shared_ptr<Path> p) { path = p; }

void Stroke::unsetSizeCalculated() { this->sizeCalculated = false; }

void Stroke::Attorney::setBoundingBoxes(Stroke& stroke, const Rectangle<double>& thinBB,
                                        const Rectangle<double>& thickBB) {
    stroke.snappedBounds = thinBB;
    stroke.Element::x = thickBB.x;
    stroke.Element::y = thickBB.y;
    stroke.Element::width = thickBB.width;
    stroke.Element::height = thickBB.height;
    stroke.sizeCalculated = true;
}

// TODO: Change the StrokeHandler's behaviour on pressure
// TODO: Adapt SplineHandler
// TODO: Stroke::rotate Stroke::scale Stroke::calcSize Stroke::isInSelection
// TODO: Stroke::scalePressure, hasPressure
// TODO: Stroke::intersects
