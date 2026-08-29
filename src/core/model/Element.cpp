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

void Element::setColor(Color color) { this->color = color; }

auto Element::getColor() const -> Color { return this->color; }

auto Element::intersectsArea(double x, double y, double width, double height) const -> bool {
    return this->getBoundingBox().intersects(xoj::util::Rectangle<double>(x, y, width, height)).has_value();
}

auto Element::hasBoundingBoxContaining(double x, double y) const -> bool {
    return Range(this->getBoundingBox()).contains(x, y);
}

auto Element::rescaleOnlyAspectRatio() const -> bool { return false; }
auto Element::rescaleWithMirror() const -> bool { return false; }

void Element::serialize(ObjectOutputStream& out) const {
    out.writeObject("Element");

    out.writeUInt(uint32_t(this->color));

    out.endObject();
}

void Element::readSerialized(ObjectInputStream& in) {
    in.readObject("Element");

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
