#include "Rectangle.h"
#include <glib.h>

Rectangle::Rectangle() {
	this->x = 0;
	this->y = 0;
	this->width = 0;
	this->height = 0;
}

Rectangle::Rectangle(double x, double y, double width, double height) {
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
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
bool Rectangle::intersect(const Rectangle * src, Rectangle * dest = NULL) {
	double destX, destY;
	double destW, destH;

	g_return_val_if_fail(src != NULL, false);

	bool returnVal = false;

	destX = MAX(this->x, src->x);
	destY = MAX(this->y, src->y);
	destW = MIN(this->x + this->width, src->x + src->width) - destX;
	destH = MIN(this->y + this->height, src->y + src->height) - destY;

	if (destW > 0 && destH > 0) {
		if (dest) {
			dest->x = destX;
			dest->y = destY;
			dest->width = destW;
			dest->height = destH;
		}
		returnVal = true;
	} else if (dest) {
		dest->width = 0;
		dest->height = 0;
	}

	return returnVal;
}
