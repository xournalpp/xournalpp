/*
 * Xournal++
 *
 * A point of a stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

class Point
{
public:
	Point();
	Point(const Point& p);
	Point(double x, double y);
	Point(double x, double y, double z);
	virtual ~Point();

public:
	double lineLengthTo(const Point& p);
	Point lineTo(const Point& p, double length);
	bool equalsPos(const Point& p);

public:
	XOJ_TYPE_ATTRIB;

	double x;
	double y;

	// pressure
	double z;


	static constexpr double NO_PRESURE = -1;
};
