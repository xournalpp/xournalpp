#include "Stroke.h"

#include <cmath>
#include <numeric>

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
    s->points = this->points;
    return s;
}

auto Stroke::clone() -> Element* { return this->cloneStroke(); }

void Stroke::serialize(ObjectOutputStream& out) {
    out.writeObject("Stroke");

    serializeAudioElement(out);

    out.writeDouble(this->width);

    out.writeInt(this->toolType);

    out.writeInt(fill);

    out.writeData(this->points.data(), this->points.size(), sizeof(Point));

    this->lineStyle.serialize(out);

    out.endObject();
}

void Stroke::readSerialized(ObjectInputStream& in) {
    in.readObject("Stroke");

    readSerializedAudioElement(in);

    this->width = in.readDouble();

    this->toolType = static_cast<StrokeTool>(in.readInt());

    this->fill = in.readInt();

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

auto Stroke::isInSelection(ShapeContainer* container) -> bool {
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
    this->sizeCalculated = false;
}

auto Stroke::getPointCount() const -> int { return this->points.size(); }

auto Stroke::getPointVector() const -> std::vector<Point> const& { return points; }

void Stroke::deletePointsFrom(int index) { points.resize(std::min(size_t(index), points.size())); }

void Stroke::deletePoint(int index) { this->points.erase(std::next(begin(this->points), index)); }

auto Stroke::getPoint(int index) const -> Point {
    if (index < 0 || index >= this->points.size()) {
        g_warning("Stroke::getPoint(%i) out of bounds!", index);
        return Point(0, 0, Point::NO_PRESSURE);
    }
    return points.at(index);
}

auto Stroke::getPoints() const -> const Point* { return this->points.data(); }

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

    this->sizeCalculated = false;
}

void Stroke::rotate(double x0, double y0, double th) {
    cairo_matrix_t rotMatrix;
    cairo_matrix_init_identity(&rotMatrix);
    cairo_matrix_translate(&rotMatrix, x0, y0);
    cairo_matrix_rotate(&rotMatrix, th);
    cairo_matrix_translate(&rotMatrix, -x0, -y0);

    for (auto&& p: points) {
        cairo_matrix_transform_point(&rotMatrix, &p.x, &p.y);
    }
    // Width and Height will likely be changed after this operation
    calcSize();
}

void Stroke::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    double fz = (restoreLineWidth) ? 1 : sqrt(fx * fy);
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
    for (auto&& p: this->points) {
        p.z *= factor;
    }
}

void Stroke::clearPressure() {
    for (auto&& p: points) {
        p.z = Point::NO_PRESSURE;
    }
}

void Stroke::setLastPressure(double pressure) {
    if (!this->points.empty()) {
        this->points.back().z = pressure;
    }
}

void Stroke::setPressure(const vector<double>& pressure) {
    // The last pressure is not used - as there is no line drawn from this point
    if (this->points.size() - 1 != pressure.size()) {
        g_warning("invalid pressure point count: %s, expected %s", std::to_string(pressure.size()).data(),
                  std::to_string(this->points.size() - 1).data());
    }

    auto max_size = std::min(pressure.size(), this->points.size() - 1);
    for (size_t i = 0U; i != max_size; ++i) {
        this->points[i].z = pressure[i];
    }
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize) -> bool {
    return intersects(x, y, halfEraserSize, nullptr);
}

/**
 * checks if the stroke is intersected by the eraser rectangle
 */
auto Stroke::intersects(double x, double y, double halfEraserSize, double* gap) -> bool {
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

    double minX = DBL_MAX;
    double maxX = DBL_MIN;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;

    double minSnapX = DBL_MAX;
    double maxSnapX = DBL_MIN;
    double minSnapY = DBL_MAX;
    double maxSnapY = DBL_MIN;

    bool hasPressure = points[0].z != Point::NO_PRESSURE;
    double halfThick = this->width / 2.0;  //  accommodate for pen width

    for (auto&& p: points) {
        if (hasPressure) {
            halfThick = p.z / 2.0;
        }

        minX = std::min(minX, p.x - halfThick);
        minY = std::min(minY, p.y - halfThick);

        maxX = std::max(maxX, p.x + halfThick);
        maxY = std::max(maxY, p.y + halfThick);

        minSnapX = std::min(minSnapX, p.x);
        minSnapY = std::min(minSnapY, p.y);

        maxSnapX = std::max(maxSnapX, p.x);
        maxSnapY = std::max(maxSnapY, p.y);
    }

    Element::x = minX;
    Element::y = minY;
    Element::width = maxX - minX;
    Element::height = maxY - minY;

    Element::snappedBounds = Rectangle<double>(minSnapX, minSnapY, maxSnapX - minSnapX, maxSnapY - minSnapY);
}

auto Stroke::getEraseable() -> EraseableStroke* { return this->eraseable; }

void Stroke::setEraseable(EraseableStroke* eraseable) { this->eraseable = eraseable; }

void Stroke::debugPrint() {
    g_message("%s", FC(FORMAT_STR("Stroke {1} / hasPressure() = {2}") % (uint64_t)this % this->hasPressure()));

    for (auto&& p: points) {
        g_message("%lf / %lf", p.x, p.y);
    }

    g_message("\n");
}
