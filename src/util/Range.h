/*
 * Xournal++
 *
 * Range
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XournalType.h"

class Range
{
public:
	Range(double x, double y);
	virtual ~Range();

	void addPoint(double x, double y);

	double getX() const;
	double getY() const;
	double getWidth() const;
	double getHeight() const;

	double getX2() const;
	double getY2() const;

private:
	double x1;
	double y1;
	double y2;
	double x2;
};
