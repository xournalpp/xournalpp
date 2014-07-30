#include "Rectangle.h"
#include "Range.h"

Rectangle::Rectangle()
{
	XOJ_INIT_TYPE(Rectangle);

	this->x = 0;
	this->y = 0;
	this->width = 0;
	this->height = 0;
}

Rectangle::Rectangle(double x, double y, double width, double height)
{
	XOJ_INIT_TYPE(Rectangle);

	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

Rectangle::Rectangle(Range& rect)
{
	XOJ_INIT_TYPE(Rectangle);

	this->x = rect.getX();
	this->y = rect.getY();
	this->width = rect.getWidth();
	this->height = rect.getHeight();
}

Rectangle::~Rectangle()
{
	XOJ_RELEASE_TYPE(Rectangle);
}


bool Rectangle::intersects(const Rectangle& other,
                           Rectangle* dest) const
{
	XOJ_CHECK_TYPE(Rectangle);

	double destX, destY;
	double destW, destH;

	bool returnVal = false;

	destX = MAX(this->x, other.x);
	destY = MAX(this->y, other.y);
	destW = MIN(this->x + this->width, other.x + other.width) - destX;
	destH = MIN(this->y + this->height, other.y + other.height) - destY;

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
	XOJ_CHECK_TYPE(Rectangle);

	if (width <= 0 || height <= 0)
	{
		return;
	}

	double x1 = MIN(this->x, x);
	double y1 = MIN(this->y, y);

	double x2 = MAX(this->x + this->width, x + width);
	double y2 = MAX(this->y + this->height, y + height);

	this->x = x1;
	this->y = y1;
	this->width = x2 - x1;
	this->height = y2 - y1;
}

void Rectangle::add(const Rectangle &other)
{
	add(other.x, other.y, other.width, other.height);
}

Rectangle Rectangle::translated(double dx, double dy)
{
	return Rectangle(this->x + dx, this->y + dy,
	                 this->width, this->height);
}

Rectangle Rectangle::intersect(const Rectangle &other)
{
	double x1 = MAX(this->x, other.x);
	double y1 = MAX(this->y, other.y);
	
	double x2 = MIN(this->x + this->width, other.x + other.width);
	double y2 = MIN(this->y + this->height, other.y + other.height);
	
	return Rectangle(x1, y1, x2 - x1, y2 - y1);
}

Rectangle& Rectangle::operator*=(double factor)
{
	x *= factor;
	y *= factor;

	width *= factor;
	height *= factor;

	return *this;
}
