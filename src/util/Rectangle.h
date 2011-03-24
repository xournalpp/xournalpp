/*
 * Xournal++
 *
 * A rectangle with double precision
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include "XournalType.h"

class Range;

class Rectangle {
public:
	Rectangle();
	Rectangle(Range & rect);
	Rectangle(double x, double y, double width, double height);
	~Rectangle();

public:

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
	bool intersect(const Rectangle * src, Rectangle * dest);

	void add(double x, double y, double width, double height);

public:
	XOJ_TYPE_ATTRIB;

	double x;
	double y;
	double width;
	double height;
};

#endif /* RECTANGLE_H_ */
