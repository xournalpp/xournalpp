/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __ERASEABLESTROKEPART_H__
#define __ERASEABLESTROKEPART_H__

#include <glib.h>
#include "../Point.h"
#include <XournalType.h>

class EraseableStrokePart {
public:
	EraseableStrokePart(Point a, Point b);
	EraseableStrokePart(double width);
	virtual ~EraseableStrokePart();

private:
	EraseableStrokePart(const EraseableStrokePart & part);

public:
	void addPoint(Point p);
	double getWidth();

	GList * getPoints();

	void clearSplitData();
	void splitFor(double halfEraserSize);

	EraseableStrokePart * clone();

	void calcSize();

public:
	double getX();
	double getY();
	double getElementWidth();
	double getElementHeight();


	static void printDebugStrokeParts();

private:
	XOJ_TYPE_ATTRIB;

	double width;
	double splitSize;

	GList * points;

	double x;
	double y;
	double elementWidth;
	double elementHeight;

	friend class EraseableStroke;
};

#endif /* __ERASEABLESTROKEPART_H__ */
