#include "Range.h"
#include <glib.h>
// TODO: AA: type check

Range::Range(double x, double y) {
	this->x1 = x;
	this->x2 = x;

	this->y1 = y;
	this->y2 = y;
}

Range::~Range() {
}

void Range::addPoint(double x, double y) {
	this->x1 = MIN(this->x1, x);
	this->x2 = MAX(this->x2, x);

	this->y1 = MIN(this->y1, y);
	this->y2 = MAX(this->y2, y);
}

double Range::getX() {
	return this->x1;
}

double Range::getY() {
	return this->y1;
}

double Range::getWidth() {
	return this->x2 - this->x1;
}

double Range::getHeight() {
	return this->y2 - this->y1;
}

double Range::getX2() {
	return this->x2;
}

double Range::getY2() {
	return this->y2;
}

