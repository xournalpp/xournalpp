#include "Element.h"

#include <algorithm>  // for max, min
#include <cinttypes>  // for uint32_t
#include <cmath>      // for ceil, floor, NAN

#include <glib.h>  // for gint

#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

Element::Element(ElementType type): type(type) {}

Element::~Element() = default;

auto Element::getType() const -> ElementType { return this->type; }

void Element::setX(double x) {
    this->x = x;
    this->sizeCalculated = false;
}

void Element::setY(double y) {
    this->y = y;
    this->sizeCalculated = false;
}

auto Element::getX() const -> double {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return x;
}

auto Element::getY() const -> double {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return y;
}
auto Element::getSnappedBounds() const -> Rectangle<double> {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return this->snappedBounds;
}

void Element::move(double dx, double dy) {
    this->x += dx;
    this->y += dy;
    this->snappedBounds = this->snappedBounds.translated(dx, dy);
}

auto Element::getElementWidth() const -> double {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return this->width;
}

auto Element::getElementHeight() const -> double {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return this->height;
}

auto Element::boundingRect() const -> Rectangle<double> {
    return Rectangle<double>(getX(), getY(), getElementWidth(), getElementHeight());
}

void Element::setColor(Color color) { this->color = color; }

auto Element::getColor() const -> Color { return this->color; }

auto Element::intersectsArea(const GdkRectangle* src) const -> bool {
    // compute the smallest rectangle with integer coordinates containing the bounding box and having width, height > 0
    auto x = getX();
    auto y = getY();
    auto x1 = gint(std::floor(getX()));
    auto y1 = gint(std::floor(getY()));
    auto x2 = gint(std::ceil(x + getElementWidth()));
    auto y2 = gint(std::ceil(y + getElementHeight()));
    GdkRectangle rect = {x1, y1, std::max(1, x2 - x1), std::max(1, y2 - y1)};

    return gdk_rectangle_intersect(src, &rect, nullptr);
}

auto Element::intersectsArea(double x, double y, double width, double height) const -> bool {
    double dest_x = NAN, dest_y = NAN;
    double dest_w = NAN, dest_h = NAN;

    dest_x = std::max(getX(), x);
    dest_y = std::max(getY(), y);
    dest_w = std::min(getX() + getElementWidth(), x + width) - dest_x;
    dest_h = std::min(getY() + getElementHeight(), y + height) - dest_y;

    return (dest_w > 0 && dest_h > 0);
}

auto Element::isInSelection(ShapeContainer* container) const -> bool {
    if (!container->contains(getX(), getY())) {
        return false;
    }
    if (!container->contains(getX() + getElementWidth(), getY())) {
        return false;
    }
    if (!container->contains(getX(), getY() + getElementHeight())) {
        return false;
    }
    if (!container->contains(getX() + getElementWidth(), getY() + getElementHeight())) {
        return false;
    }

    return true;
}

auto Element::rescaleOnlyAspectRatio() -> bool { return false; }
auto Element::rescaleWithMirror() -> bool { return false; }

void Element::serialize(ObjectOutputStream& out) const {
    out.writeObject("Element");

    out.writeDouble(this->x);
    out.writeDouble(this->y);
    out.writeInt(int(uint32_t(this->color)));

    out.endObject();
}

void Element::readSerialized(ObjectInputStream& in) {
    in.readObject("Element");

    this->x = in.readDouble();
    this->y = in.readDouble();
    this->color = Color(in.readInt());

    in.endObject();
}
