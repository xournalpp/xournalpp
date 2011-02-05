#include "Point.h"

#include <math.h>
#include <stdio.h>

Point::Point() {
	this->x = 0;
	this->y = 0;
	this->z = NO_PRESURE;
}

Point::Point(double x, double y) {
	this->x = x;
	this->y = y;
	this->z = NO_PRESURE;
}

Point::Point(double x, double y, double z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

double Point::lineLengthTo(const Point p) {
	return hypot(this->x - p.x, this->y - p.y);
}

Point Point::lineTo(const Point p, double length) {
	double factor = lineLengthTo(p);
	factor = length / factor;

	printf("factor = %lf\n", factor);

	double x = p.x - this->x;
	double y = p.y - this->y;
	x *= factor;
	y *= factor;
	x += this->x;
	y += this->y;

	return Point(x, y);
}

