#include "util/Range.h"

#include <algorithm>  // for max, min

#include "util/Rectangle.h"


Range::Range(const xoj::util::Rectangle<double>& r): minX(r.x), minY(r.y), maxX(r.x + r.width), maxY(r.y + r.height) {}

void Range::addPoint(double x, double y) {
    this->minX = std::min(this->minX, x);
    this->maxX = std::max(this->maxX, x);

    this->minY = std::min(this->minY, y);
    this->maxY = std::max(this->maxY, y);
}

Range Range::unite(const Range& o) const {
    return Range(std::min(minX, o.minX), std::min(minY, o.minY), std::max(maxX, o.maxX), std::max(maxY, o.maxY));
}

Range Range::intersect(const Range& o) const {
    Range rg(std::max(minX, o.minX), std::max(minY, o.minY), std::min(maxX, o.maxX), std::min(maxY, o.maxY));
    return rg.isValid() ? rg : Range();
}

auto Range::getX() const -> double { return this->minX; }

auto Range::getY() const -> double { return this->minY; }

auto Range::getWidth() const -> double { return this->maxX - this->minX; }

auto Range::getHeight() const -> double { return this->maxY - this->minY; }

void Range::addPadding(double padding) {
    this->minX -= padding;
    this->maxX += padding;
    this->minY -= padding;
    this->maxY += padding;
}

void Range::translate(double dx, double dy) {
    this->minX += dx;
    this->maxX += dx;
    this->minY += dy;
    this->maxY += dy;
}

auto Range::empty() const -> bool {
    Range empty;
    return this->minX == empty.minX && this->minY == empty.minY && this->maxX == empty.maxX && this->maxY == empty.maxY;
}

bool Range::isValid() const { return minX <= maxX && minY <= maxY; }

bool Range::contains(double x, double y) const { return x >= minX && x <= maxX && y >= minY && y <= maxY; }

bool Range::contains(const xoj::util::Rectangle<double>& r) const {
    return this->minX <= r.x && this->maxX >= r.x + r.width && this->minY <= r.y && this->maxY >= r.y + r.height;
}
