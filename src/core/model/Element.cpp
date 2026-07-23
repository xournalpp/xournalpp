#include "Element.h"

#include <algorithm>  // for max, min
#include <cmath>      // for ceil, floor, NAN
#include <cstdint>    // for uint32_t

#include <glib.h>  // for gint

#include "util/Point.h"
#include "util/safe_casts.h"                      // for as_unsigned
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

Element::Element(ElementType type): type(type) {}

auto Element::getType() const -> ElementType { return this->type; }

auto Element::getBoundingBox() const -> const Rectangle<double>& {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return this->boundingBox;
}

auto Element::getSnappedBounds() const -> const Rectangle<double>& {
    if (!this->sizeCalculated) {
        this->sizeCalculated = true;
        calcSize();
    }
    return this->snappedBounds;
}

void Element::setOrigin(double x, double y) {
    this->boundingBox.x = x;
    this->boundingBox.y = y;
    this->sizeCalculated = false;
}

auto Element::getOrigin() const -> const xoj::util::Point<double>& { return boundingBox.getOrigin(); }

void Element::move(double dx, double dy) {
    this->boundingBox = this->boundingBox.translated(dx, dy);
    this->snappedBounds = this->snappedBounds.translated(dx, dy);
}

void Element::setColor(Color color) { this->color = color; }

auto Element::getColor() const -> Color { return this->color; }

auto Element::intersectsArea(double x, double y, double width, double height) const -> bool {
    return this->getBoundingBox().intersects(xoj::util::Rectangle<double>(x, y, width, height)).has_value();
}

auto Element::distanceTo(double x, double y) const -> double {
    const auto& box = this->getBoundingBox();
    // Coordinates of the point in the bounding box that is the closest to (x,y).
    double projX = std::clamp(x, box.x, box.x + box.width);
    double projY = std::clamp(y, box.y, box.y + box.height);
    return std::hypot(x - projX, y - projY);
}

auto Element::hasBoundingBoxContaining(double x, double y) const -> bool {
    return Range(this->getBoundingBox()).contains(x, y);
}

auto Element::isInSelection(ShapeContainer* container) const -> bool {
    const auto& box = this->getBoundingBox();
    if (!container->contains(box.x, box.y)) {
        return false;
    }
    if (!container->contains(box.x + box.width, box.y)) {
        return false;
    }
    if (!container->contains(box.x, box.y + box.height)) {
        return false;
    }
    if (!container->contains(box.x + box.width, box.y + box.height)) {
        return false;
    }

    return true;
}

auto Element::rescaleOnlyAspectRatio() const -> bool { return false; }
auto Element::rescaleWithMirror() const -> bool { return false; }

void Element::serialize(ObjectOutputStream& out) const {
    out.writeObject("Element");

    const auto& pt = getOrigin();
    out.writeDouble(pt.x);
    out.writeDouble(pt.y);
    out.writeUInt(uint32_t(this->color));

    out.endObject();
}

void Element::readSerialized(ObjectInputStream& in) {
    in.readObject("Element");

    double x = in.readDouble();
    double y = in.readDouble();
    setOrigin(x, y);
    this->color = Color(in.readUInt());

    in.endObject();
}

namespace xoj {

auto refElementContainer(const std::vector<ElementPtr>& elements) -> std::vector<Element*> {
    std::vector<Element*> result(elements.size());
    std::transform(elements.begin(), elements.end(), result.begin(), [](auto const& e) { return e.get(); });
    return result;
}

}  // namespace xoj
