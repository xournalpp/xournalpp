#include "Element.h"

#include <algorithm>  // for max, min
#include <cstdint>    // for uint32_t
#include <iostream>
#include <system_error>

#include <glib.h>  // for gint

#include "util/Matrix.h"
#include "util/Point.h"
#include "util/Rectangle.h"
#include "util/serializing/ObjectInputStream.h"   // for ObjectInputStream
#include "util/serializing/ObjectOutputStream.h"  // for ObjectOutputStream

using xoj::util::Rectangle;

Element::Element(ElementType type): type(type) {}

void Element::move(double dx, double dy) { this->transformation = this->transformation.translate(dx, dy); }

void Element::scale(xoj::util::Point<double> base, double fx, double fy, bool restoreLineWidth) {
    this->transformation = this->transformation.translate(-base.x, -base.y).scale(fx, fy).translate(base.x, base.y);
}

void Element::rotate(xoj::util::Point<double> base, double th) {
    this->transformation = this->transformation.translate(-base.x, -base.y).rotate(th).translate(base.x, base.y);
}

void Element::setX(double x) {
    auto x_orig = boundingRect().x;
    auto tx = x - x_orig;
    this->transformation.tx += tx;
}

void Element::setY(double y) {
    auto y_orig = boundingRect().y;
    auto ty = y - y_orig;
    this->transformation.ty += ty;
}

void Element::setWidth(double width) {
    auto factor = width / this->getElementWidth();
    this->scale({0, 0}, factor, 1, false);
}

void Element::setHeight(double height) {
    auto factor = height / this->getElementHeight();
    this->scale({0, 0}, 1, factor, false);
}

auto Element::getX() const -> double { return boundingRect().x; }

auto Element::getY() const -> double { return boundingRect().y; }

auto Element::getElementWidth() const -> double { return boundingRect().width; }

auto Element::getElementHeight() const -> double { return boundingRect().height; }

auto Element::boundingRect() const -> Rectangle<double> { return updateBounds().first; }

auto Element::getSnappedBounds() const -> Rectangle<double> { return updateBounds().second; }

void Element::setColor(Color color) { this->color = color; }

auto Element::getColor() const -> Color { return this->color; }

auto Element::getType() const -> ElementType { return this->type; }

auto Element::intersectsArea(const GdkRectangle* src) const -> bool {
    return intersectsArea(Rectangle<double>(src->x, src->y, src->width, src->height));
}

auto Element::intersectsArea(double x, double y, double width, double height) const -> bool {
    return intersectsArea(Rectangle<double>(x, y, width, height));
}

auto Element::intersectsArea(Rectangle<double> bounds) const -> bool {
    return bounds.intersects(boundingRect()).has_value();
}

auto Element::updateBounds() const -> std::pair<Rectangle<double>, Rectangle<double>> {
    auto [childBounds, snappedBounds] = internalUpdateBounds();
    return {this->transformation * childBounds, this->transformation * snappedBounds};
}

auto Element::isInSelection(ShapeContainer* container) const -> bool {
    auto rect = boundingRect();
    auto x2 = rect.x + rect.width;
    auto y2 = rect.y + rect.height;
    return container->contains(rect.x, rect.y) ||  //
           container->contains(x2, rect.y) ||      //
           container->contains(rect.x, y2) ||      //
           container->contains(x2, y2);
}

auto Element::rescaleOnlyAspectRatio() -> bool { return false; }
auto Element::rescaleWithMirror() -> bool { return false; }

void Element::serialize(ObjectOutputStream& out) const {
    out.writeObject("Element");

    out.writeDouble(this->transformation.tx);
    out.writeDouble(this->transformation.ty);
    // Todo write the transformation matrix, versioning
    out.writeUInt(uint32_t(this->color));
    out.endObject();
}

void Element::readSerialized(ObjectInputStream& in) {
    in.readObject("Element");

    this->transformation.tx = in.readDouble();
    this->transformation.ty = in.readDouble();
    // Todo read the transformation matrix, versioning
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

auto Element::getTransformation() const -> xoj::util::Matrix { return this->transformation; }

void Element::setTransformation(xoj::util::Matrix const& mtx) { this->transformation = mtx; }
