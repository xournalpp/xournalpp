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


/**
 * @src1: a #Rectangle
 * @src2: a #Rectangle
 * @dest: return location for the intersection of @src1 and @src2, or %NULL
 *
 * Calculates the intersection of two rectangles. It is allowed for
 * @dest to be the same as either @src1 or @src2. If the rectangles
 * do not intersect, @dest's width and height is set to 0 and its x
 * and y values are undefined. If you are only interested in whether
 * the rectangles intersect, but not in the intersecting area itself,
 * pass %NULL for @dest.
 *
 * Returns: %TRUE if the rectangles intersect.
 */
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

