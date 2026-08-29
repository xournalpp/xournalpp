#include "RectangularElement.h"

#include <algorithm>  // for max, min
#include <cmath>      // for ceil, floor, NAN
#include <cstdint>    // for uint32_t

#include "util/Point.h"
#include "util/safe_casts.h"                      // for as_unsigned
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

static constexpr auto SERIALIZATION_NAME = "RectangularElement";

RectangularElement::RectangularElement(ElementType type): Element(type) {}

void RectangularElement::setTransformation(const xoj::util::Matrix& m) {
    this->transformationMatrix = m;
    this->sizeCalculated = false;
}

void RectangularElement::move(double dx, double dy) {
    this->boundingBox = this->boundingBox.translated(dx, dy);
    this->snappedBounds = this->snappedBounds.translated(dx, dy);
    this->transformationMatrix = xoj::util::Matrix::TRANSLATION(dx, dy) * this->transformationMatrix;
}

void RectangularElement::scale(double x0, double y0, double fx, double fy, double rotation, bool restoreLineWidth) {
    this->transformationMatrix = xoj::util::Matrix::TRANSLATION(x0, y0) * xoj::util::Matrix::ROTATION(rotation) *
                                 xoj::util::Matrix::SCALING(fx, fy) * xoj::util::Matrix::ROTATION(-rotation) *
                                 xoj::util::Matrix::TRANSLATION(-x0, -y0) * this->transformationMatrix;
    this->sizeCalculated = false;
}

void RectangularElement::rotate(double x0, double y0, double th) {
    this->transformationMatrix = xoj::util::Matrix::TRANSLATION(x0, y0) * xoj::util::Matrix::ROTATION(th) *
                                 xoj::util::Matrix::TRANSLATION(-x0, -y0) * this->transformationMatrix;
    this->sizeCalculated = false;
}

auto RectangularElement::distanceTo(double x, double y) const -> double {
    auto p = this->transformationMatrix.inverse() * xoj::util::Point<double>(x, y);
    // Coordinates of the point in the rectangle that is the closest to p, in local Element coordinates
    xoj::util::Point<double> proj(std::clamp(p.x, 0., naturalSize.width), std::clamp(p.y, 0., naturalSize.height));
    // Go back to page coordinates and compute the distance
    return (this->transformationMatrix * proj).distance({x, y});
}

auto RectangularElement::isInSelection(ShapeContainer* container) const -> bool {
    // We consider a rectangular element is selected if its 4 corners are selected.
    // This is not perfect and could give counterintuitive results in corner cases when using the lasso
    const auto& origin = this->transformationMatrix.shift;
    const auto v1 = this->transformationMatrix * xoj::util::Point<double>{0., naturalSize.height};
    const auto v2 = this->transformationMatrix * xoj::util::Point<double>{naturalSize.width, 0.};
    for (auto&& p: {origin, v1, v2, v1 + v2 - origin}) {
        // The 4 corners of the parallelogram cutting out our element (after transformation)
        if (!container->contains(p.x, p.y)) {
            return false;
        }
    }
    return true;
}

auto RectangularElement::getNaturalSize() const -> const xoj::util::Size<double>& {
    if (!sizeCalculated) {
        calcSize();
    }
    return naturalSize;
}

void RectangularElement::serialize(ObjectOutputStream& out) const {
    out.writeObject(SERIALIZATION_NAME);

    Element::serialize(out);

    out.writeDouble(transformationMatrix.xx);
    out.writeDouble(transformationMatrix.yx);
    out.writeDouble(transformationMatrix.xy);
    out.writeDouble(transformationMatrix.yy);
    out.writeDouble(transformationMatrix.shift.x);
    out.writeDouble(transformationMatrix.shift.y);
    out.writeDouble(naturalSize.width);
    out.writeDouble(naturalSize.height);

    out.endObject();
}

void RectangularElement::readSerialized(ObjectInputStream& in) {
    in.readObject(SERIALIZATION_NAME);

    Element::readSerialized(in);

    xoj::util::Matrix m;
    m.xx = in.readDouble();
    m.yx = in.readDouble();
    m.xy = in.readDouble();
    m.yy = in.readDouble();
    m.shift.x = in.readDouble();
    m.shift.y = in.readDouble();
    setTransformation(m);
    naturalSize.width = in.readDouble();
    naturalSize.height = in.readDouble();

    in.endObject();
}
