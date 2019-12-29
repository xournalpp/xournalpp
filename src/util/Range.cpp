#include "Range.h"

Range::Range(double x, double y) {
    this->x1 = x;
    this->x2 = x;

    this->y1 = y;
    this->y2 = y;
}

Range::~Range() = default;

void Range::addPoint(double x, double y) {
    this->x1 = std::min(this->x1, x);
    this->x2 = std::max(this->x2, x);

    this->y1 = std::min(this->y1, y);
    this->y2 = std::max(this->y2, y);
}

auto Range::getX() const -> double { return this->x1; }

auto Range::getY() const -> double { return this->y1; }

auto Range::getWidth() const -> double { return this->x2 - this->x1; }

auto Range::getHeight() const -> double { return this->y2 - this->y1; }

auto Range::getX2() const -> double { return this->x2; }

auto Range::getY2() const -> double { return this->y2; }
