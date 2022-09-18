#include "LegacyRedrawable.h"

#include "model/Element.h"  // for Element
#include "util/Range.h"     // for Range

void LegacyRedrawable::repaintElement(Element* e) const {
    repaintArea(e->getX(), e->getY(), e->getElementWidth() + e->getX(), e->getElementHeight() + e->getY());
}

void LegacyRedrawable::repaintRect(double x, double y, double width, double height) const {
    repaintArea(x, y, x + width, y + height);
}

void LegacyRedrawable::rerenderRange(Range& r) { rerenderRect(r.getX(), r.getY(), r.getWidth(), r.getHeight()); }

void LegacyRedrawable::rerenderElement(Element* e) {
    rerenderRect(e->getX() - 1, e->getY() - 1, e->getElementWidth() + 2, e->getElementHeight() + 2);
}
