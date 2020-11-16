#include "Redrawable.h"

#include "model/Element.h"

void Redrawable::repaintRange(Range& r) { repaintArea(r.getX(), r.getY(), r.getX2(), r.getY2()); }

void Redrawable::repaintElement(Element* e) {
    repaintArea(e->getX(), e->getY(), e->getElementWidth() + e->getX(), e->getElementHeight() + e->getY());
}

void Redrawable::repaintRect(double x, double y, double width, double height) {
    repaintArea(x, y, x + width, y + height);
}

void Redrawable::rerenderRange(Range& r) { rerenderRect(r.getX(), r.getY(), r.getWidth(), r.getHeight()); }

void Redrawable::rerenderArea(double x1, double y1, double x2, double y2) { rerenderRect(x1, y1, x2 - x1, y2 - y1); }

void Redrawable::rerenderElement(Element* e) {
    rerenderRect(e->getX() - 1, e->getY() - 1, e->getElementWidth() + 2, e->getElementHeight() + 2);
}
