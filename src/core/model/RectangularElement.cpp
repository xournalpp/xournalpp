#include "RectangularElement.h"

#include <algorithm>  // for clamp

#include "util/Assert.h"
#include "util/Point.h"
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

static constexpr auto SERIALIZATION_NAME = "RectangularElement";

RectangularElement::RectangularElement(ElementType type): Element(type) {}

void RectangularElement::setOrigin(double x, double y) {
    this->snappedBounds.x = x;
    this->snappedBounds.y = y;
    this->sizeCalculated = false;
}

void RectangularElement::move(double dx, double dy) {
    this->boundingBox = this->boundingBox.translated(dx, dy);
    this->snappedBounds = this->snappedBounds.translated(dx, dy);
}

void RectangularElement::scale(double x0, double y0, double fx, double fy, double rotation, bool) {
    xoj_assert(rotation == 0);
    this->snappedBounds.x = (this->snappedBounds.x - x0) * fx + x0;
    this->snappedBounds.y = (this->snappedBounds.y - y0) * fy + y0;
    this->snappedBounds.width *= fx;
    this->snappedBounds.height *= fy;
    this->sizeCalculated = false;
}

void RectangularElement::rotate(double x0, double y0, double th) {}

auto RectangularElement::distanceTo(double x, double y) const -> double {
    // Coordinates of the point in the rectangle that is the closest to (x,y)
    xoj::util::Point<double> proj(std::clamp(x, snappedBounds.x, snappedBounds.x + snappedBounds.width),
                                  std::clamp(y, snappedBounds.y, snappedBounds.y + snappedBounds.height));
    return proj.distance({x, y});
}

auto RectangularElement::isInSelection(ShapeContainer* container) const -> bool {
    // We consider a rectangular element is selected if its 4 corners are selected.
    // This is not perfect and could give counterintuitive results in corner cases when using the lasso
    const auto& origin = this->snappedBounds.getOrigin();
    const auto v1 = origin + xoj::util::Point<double>{0., snappedBounds.height};
    const auto v2 = origin + xoj::util::Point<double>{snappedBounds.width, 0.};
    for (auto&& p: {origin, v1, v2, v1 + v2 - origin}) {
        // The 4 corners of the rectangle cutting out our element
        if (!container->contains(p.x, p.y)) {
            return false;
        }
    }
    return true;
}

void RectangularElement::serialize(ObjectOutputStream& out) const {
    out.writeObject(SERIALIZATION_NAME);

    Element::serialize(out);

    out.writeDouble(snappedBounds.x);
    out.writeDouble(snappedBounds.y);

    out.endObject();
}

void RectangularElement::readSerialized(ObjectInputStream& in) {
    in.readObject(SERIALIZATION_NAME);

    Element::readSerialized(in);

    snappedBounds.x = in.readDouble();
    snappedBounds.y = in.readDouble();

    in.endObject();
}
