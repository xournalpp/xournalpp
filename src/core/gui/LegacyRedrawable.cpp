#include "LegacyRedrawable.h"

#include "model/Element.h"  // for Element
#include "util/Range.h"     // for Range

void LegacyRedrawable::repaintElement(const Element* e) const {
    Range r(e->getBoundingBox());
    repaintArea(r.minX, r.minY, r.maxX, r.maxY);
}

void LegacyRedrawable::repaintRect(double x, double y, double width, double height) const {
    repaintArea(x, y, x + width, y + height);
}

void LegacyRedrawable::rerenderRange(const Range& r) { rerenderRect(r.getX(), r.getY(), r.getWidth(), r.getHeight()); }

void LegacyRedrawable::rerenderElement(const Element* e) {
    const auto& rect = e->getBoundingBox();
    rerenderRect(rect.x, rect.y, rect.width, rect.height);
}
