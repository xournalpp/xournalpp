/*
 * Xournal++
 *
 * A stroke wich is temporary used if you erase a part
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef ERASEABLESTROKE_H_
#define ERASEABLESTROKE_H_

#include <glib.h>
#include "Point.h"

class Stroke;
class EraseableStrokePart;

class EraseableStroke {
public:
	EraseableStroke(Stroke * stroke);
	virtual ~EraseableStroke();

public:
	bool erase(double x, double y, double halfEraserSize);
	GList * getParts();
	GList * getStroke(Stroke * original);

private:
	bool erase(double x, double y, double halfEraserSize, EraseableStrokePart * part);
	bool erasePart(double x, double y, double halfEraserSize, EraseableStrokePart * part);

private:

	GList * parts;
};

class EraseableStrokePart {
public:
	EraseableStrokePart(Point a, Point b);
	EraseableStrokePart(double width);
	~EraseableStrokePart();
public:
	void addPoint(Point p);
	double getWidth();

	GList * getPoints();

	void clearSplitData();
	void splitFor(double halfEraserSize);

private:
	double width;
	double splitSize;

	GList * points;

	friend class EraseableStroke;
};

#endif /* ERASEABLESTROKE_H_ */
