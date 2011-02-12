/*
 * Xournal++
 *
 * A point of a stroke
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __POINT_H__
#define __POINT_H__

class Point {
public:
	Point();
	Point(const Point & p);
	Point(double x, double y);
	Point(double x, double y, double z);

public:
	double lineLengthTo(const Point p);
	Point lineTo(const Point p, double length);
	bool equalsPos(const Point p);

public:
	double x;
	double y;

	// pressure
	double z;


	static const double NO_PRESURE = -1;
};
#endif /* __POINT_H__ */
