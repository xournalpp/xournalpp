#include "Range.h"

Range::Range(double x, double y)
{
	this->x1 = x;
	this->x2 = x;

	this->y1 = y;
	this->y2 = y;
}

Range::~Range()
{
}

void Range::addPoint(double x, double y)
{
	this->x1 = std::min(this->x1, x);
	this->x2 = std::max(this->x2, x);

	this->y1 = std::min(this->y1, y);
	this->y2 = std::max(this->y2, y);
}

double Range::getX() const
{
	return this->x1;
}

double Range::getY() const
{
	return this->y1;
}

double Range::getWidth() const
{
	return this->x2 - this->x1;
}

double Range::getHeight() const
{
	return this->y2 - this->y1;
}

double Range::getX2() const
{
	return this->x2;
}

double Range::getY2() const
{
	return this->y2;
}
