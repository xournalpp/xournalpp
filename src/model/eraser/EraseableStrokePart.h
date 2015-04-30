/*
 * Xournal++
 *
 * A stroke which is temporary used if you erase a part
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/Point.h"

#include <XournalType.h>

#include <glib.h>

class EraseableStrokePart
{
public:
	EraseableStrokePart(Point a, Point b);
	EraseableStrokePart(double width);
	virtual ~EraseableStrokePart();

private:
	EraseableStrokePart(const EraseableStrokePart& part);

public:
	void addPoint(Point p);
	double getWidth();

	GList* getPoints();

	void clearSplitData();
	void splitFor(double halfEraserSize);

	EraseableStrokePart* clone();

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

	GList* points;

	double x;
	double y;
	double elementWidth;
	double elementHeight;

	friend class EraseableStroke;
};
