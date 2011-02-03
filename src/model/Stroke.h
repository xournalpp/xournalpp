/*
 * Xournal++
 *
 * An element on the document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __STROKE_H__
#define __STROKE_H__

#include "Element.h"
#include "../util/Arrayiterator.h"

class Point {
public:
	Point();
	Point(double x, double y);
	Point(double x, double y, double z);

	double x;
	double y;

	// pressure
	double z;


	static const double NO_PRESURE = -1;
};

enum StrokeTool {
	STROKE_TOOL_PEN, STROKE_TOOL_ERASER, STROKE_TOOL_HIGHLIGHTER
};

class Stroke: public Element {
public:
	Stroke();
	virtual ~Stroke();

	Stroke * clone() const;

	void setWidth(double width);
	double getWidth() const;

	void addPoint(Point p);
	int getPointCount() const;
	void freeUnusedPointItems();
	ArrayIterator<Point> pointIterator() const;
	Point getPoint(int index) const;
	const Point * getPoints() const;

	void setToolType(StrokeTool type);
	StrokeTool getToolType() const;

	const double * getWidths() const;

	bool intersects(double x, double y, double halfSize, double * gap = NULL);
	Stroke * splitOnLastIntersects();

	void setPressure(const double * data);
	void setLastPressure(double pressure);
	void clearPressure();
	void scalePressure(double factor);

	bool hasPressure();

	void setCopyed(bool copyed);
	bool isCopyed();

	virtual void move(double dx, double dy);
	virtual void finalizeMove();

	virtual bool isInSelection(ShapeContainer * container);

	bool isMoved();
	double getDx();
	double getDy();

public:
	// Serialize interface
	void serialize(ObjectOutputStream & out);
	void readSerialized(ObjectInputStream & in) throw (InputStreamException);

protected:
	virtual void calcSize();
	void allocPointSize(int size);
private:
	// The stroke width
	double width;

	StrokeTool toolType;

	bool moved;

	double dx;
	double dy;

	// Split point for eraser
	int splitIndex;
	bool copyed;

	// The array with the points
	Point * points;
	int pointCount;
	int pointAllocCount;
};

#endif /* __STROKE_H__ */
