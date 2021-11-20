#include "Point.h"

#include <cmath>

Point::Point(double x, double y): x(x), y(y) {}

Point::Point(double x, double y, double z): x(x), y(y), z(z) {}

auto Point::lineLengthTo(const Point& p) const -> double { return std::hypot(this->x - p.x, this->y - p.y); }

auto Point::lineTo(const Point& p, double length) const -> Point {
    double factor = lineLengthTo(p);
    factor = length / factor;

    double x = p.x - this->x;
    double y = p.y - this->y;
    x *= factor;
    y *= factor;
    x += this->x;
    y += this->y;

    return Point(x, y);
}

auto Point::equalsPos(const Point& p) const -> bool { return this->x == p.x && this->y == p.y; }
