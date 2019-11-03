#include "Point.h"

#include <cmath>

Point::Point(double x, double y)
 : x(x)
 , y(y)
{
}

Point::Point(double x, double y, double z)
 : x(x)
 , y(y)
 , z(z)
{
}

double Point::lineLengthTo(const Point& p) const
{
	return std::hypot(this->x - p.x, this->y - p.y);
}

double Point::slopeTo(const Point& p)
{
	return std::atan2(this->x - p.x, this->y - p.y);
}

Point Point::lineTo(const Point& p, double length)
{
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

bool Point::equalsPos(const Point& p)
{
	return this->x == p.x && this->y == p.y;
}
