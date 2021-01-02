#include "Point.h"

#include <cmath>

Point::Point(double x, double y): x(x), y(y) {}

Point::Point(double x, double y, double z): x(x), y(y), z(z) {}

auto Point::lineLengthTo(const Point& p) const -> double { return std::hypot(this->x - p.x, this->y - p.y); }

auto Point::lineTo(const Point& p, double length) const -> Point { return relativeLineTo(p, length / lineLengthTo(p)); }

auto Point::relativeLineTo(const Point& p, const double ratio) const -> Point {
    const double oneMinusRatio = 1 - ratio;
    return Point(oneMinusRatio * x + ratio * p.x, oneMinusRatio * y + ratio * p.y, oneMinusRatio * z + ratio * p.z);
}

auto Point::equalsPos(const Point& p) const -> bool { return this->x == p.x && this->y == p.y; }
