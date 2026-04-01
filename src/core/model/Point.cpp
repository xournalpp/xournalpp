#include "Point.h"

#include <cmath>  // for hypot

#include "util/Rectangle.h"  // for Rectangle

Point::Point(double x, double y): x(x), y(y) {}

Point::Point(double x, double y, double z): x(x), y(y), z(z) {}

auto Point::lineLengthTo(const Point& p) const -> double { return std::hypot(this->x - p.x, this->y - p.y); }

auto Point::lineTo(const Point& p, double length) const -> Point { return relativeLineTo(p, length / lineLengthTo(p)); }

auto Point::relativeLineTo(const Point& p, const double ratio) const -> Point {
    return Point(x + ratio * (p.x - x), y + ratio * (p.y - y), z + ratio * (p.z - z));
}

auto Point::equalsPos(const Point& p) const -> bool { return this->x == p.x && this->y == p.y; }

auto Point::isInside(const xoj::util::Rectangle<double>& rect) const -> bool {
    return x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height;
}
