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

	double x;
	double y;
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

	void addWidthValue(double value);
	int getWidthCount() const;
	void freeUnusedWidthItems();
	ArrayIterator<double> widthIterator() const;
	void clearWidths();

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

	void setCopyed(bool copyed);
	bool isCopyed();

	virtual void move(double dx, double dy);
	virtual void finalizeMove();

	virtual bool isInSelection(ShapeContainer * container);

	bool isMoved();
	double getDx();
	double getDy();
protected:
	virtual void calcSize();
	void allocPointSize(int size);
	void allocWidthSize(int size);
private:
	// The stroke width
	double width;

	// The array with the thinkess variable width is activated
	double * widths;
	int widthCount;
	int widthAllocCount;

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
