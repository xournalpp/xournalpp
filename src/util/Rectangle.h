/*
 * Xournal++
 *
 * A rectangle with double precision
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __RECTANGLE_H__
#define __RECTANGLE_H__

#include <glib.h>
#include <XournalType.h>

class Range;

class Rectangle
{
public:
	Rectangle();
	Rectangle(Range& rect);
	Rectangle(double x, double y, double width, double height);
	virtual ~Rectangle();

public:

	/**
	 * Returns whether this rectangle intersects another
	 *
	 * @param other the other rectangle
	 * @param dest  if this is not NULL, the rectangle will
	 *              be modified to contain the intersection
	 *
	 * @return whether the rectangles intersect
	 */
	bool intersects(const Rectangle& other,
					Rectangle* dest = NULL) const;

	/**
	 * Computes the union of this rectangle with the one
	 * given by the parameters
	 */
	void add(double x, double y, double width, double height);

	/**
	 * Same as the above, provided for convenience
	 */
	void add(const Rectangle &other);

public:
	XOJ_TYPE_ATTRIB;

	double x;
	double y;
	double width;
	double height;
};

#endif /* RECTANGLE_H_ */
