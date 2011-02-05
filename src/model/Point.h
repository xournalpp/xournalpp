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
	Point(double x, double y);
	Point(double x, double y, double z);

public:
	double lineLengthTo(const Point p);
	Point lineTo(const Point p, double length);

public:
	double x;
	double y;

	// pressure
	double z;


	static const double NO_PRESURE = -1;
};
#endif /* __POINT_H__ */
