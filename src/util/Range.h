/*
 * Xournal++
 *
 * Range
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __RANGE_H__
#define __RANGE_H__

#include <XournalType.h>

class Range {
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

#endif /* __RANGE_H__ */
