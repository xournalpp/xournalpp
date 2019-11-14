#include "Rectangle.h"

#include "Range.h"

Rectangle::Rectangle() = default;

Rectangle::Rectangle(double x, double y, double width, double height)
 : x(x)
 , y(y)
 , width(width)
 , height(height)
{
}

Rectangle::Rectangle(const Range& rect)
 : x(rect.getX())
 , y(rect.getY())
 , width(rect.getWidth())
 , height(rect.getHeight())
{
}

Rectangle::~Rectangle() = default;

auto Rectangle::intersects(const Rectangle& other, Rectangle* dest) const -> bool
{
	double destX, destY;
	double destW, destH;

	bool returnVal = false;

	destX = std::max(this->x, other.x);
	destY = std::max(this->y, other.y);
	destW = std::min(this->x + this->width, other.x + other.width) - destX;
	destH = std::min(this->y + this->height, other.y + other.height) - destY;

	if (destW > 0 && destH > 0)
	{
		if (dest)
		{
			dest->x = destX;
			dest->y = destY;
			dest->width = destW;
			dest->height = destH;
		}
		returnVal = true;
	}
	else if (dest)
	{
		dest->width = 0;
		dest->height = 0;
	}

	return returnVal;
}

void Rectangle::add(double x, double y, double width, double height)
{
	if (width <= 0 || height <= 0)
	{
		return;
	}

	double x1 = std::min(this->x, x);
	double y1 = std::min(this->y, y);

	double x2 = std::max(this->x + this->width, x + width);
	double y2 = std::max(this->y + this->height, y + height);

	this->x = x1;
	this->y = y1;
	this->width = x2 - x1;
	this->height = y2 - y1;
}

void Rectangle::add(const Rectangle &other)
{
	add(other.x, other.y, other.width, other.height);
}

auto Rectangle::translated(double dx, double dy) -> Rectangle
{
	return Rectangle(this->x + dx, this->y + dy, this->width, this->height);
}

auto Rectangle::intersect(const Rectangle& other) -> Rectangle
{
	double x1 = std::max(this->x, other.x);
	double y1 = std::max(this->y, other.y);

	double x2 = std::min(this->x + this->width, other.x + other.width);
	double y2 = std::min(this->y + this->height, other.y + other.height);

	return Rectangle(x1, y1, x2 - x1, y2 - y1);
}

auto Rectangle::operator*=(double factor) -> Rectangle&
{
	x *= factor;
	y *= factor;

	width *= factor;
	height *= factor;

	return *this;
}

auto Rectangle::area() const -> double
{
	return width * height;
}
