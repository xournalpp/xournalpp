/*
 * Xournal++
 *
 * Range
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#pragma once

#include <XournalType.h>

class Range
{
public:
	Range(double x, double y);
	virtual ~Range();

	void addPoint(double x, double y);

	double getX();
	double getY();
	double getWidth();
	double getHeight();

	double getX2();
	double getY2();

private:
	XOJ_TYPE_ATTRIB;

	double x1;
	double y1;
	double y2;
	double x2;
};
