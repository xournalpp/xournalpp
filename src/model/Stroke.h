/*
 * Xournal++
 *
 * A stroke on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"
#include "Point.h"

#include <Arrayiterator.h>

enum StrokeTool
{
	STROKE_TOOL_PEN, STROKE_TOOL_ERASER, STROKE_TOOL_HIGHLIGHTER
};

class EraseableStroke;

class Stroke : public Element
{
public:
	Stroke();
	virtual ~Stroke();

public:
	Stroke* cloneStroke() const;
	virtual Element* clone();

	void setWidth(double width);
	double getWidth() const;

	void setTimestamp(int seconds);
	int getTimestamp() const;

	void setAudioFilename(string fn);
	string getAudioFilename() const;

	void addPoint(Point p);
	void setLastPoint(double x, double y);
	void setLastPoint(Point p);
	int getPointCount() const;
	void freeUnusedPointItems();
	ArrayIterator<Point> pointIterator() const;
	Point getPoint(int index) const;
	const Point* getPoints() const;

	void deletePoint(int index);
	void deletePointsFrom(int index);

	void setToolType(StrokeTool type);
	StrokeTool getToolType() const;

	const double* getWidths() const;

	bool intersects(double x, double y, double halfSize, double* gap = NULL);

	void setPressure(const double* data);
	void setLastPressure(double pressure);
	void clearPressure();
	void scalePressure(double factor);

	bool hasPressure();

	virtual void move(double dx, double dy);
	virtual void scale(double x0, double y0, double fx, double fy);
	void rotate(double x0, double y0, double th);

	virtual bool isInSelection(ShapeContainer* container);

	EraseableStroke* getEraseable();
	void setEraseable(EraseableStroke* eraseable);

	void debugPrint();

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in) throw (InputStreamException);

protected:
	virtual void calcSize();
	void allocPointSize(int size);

private:
	XOJ_TYPE_ATTRIB;

	// The stroke width cannot be inherited from Element
	double width;

	StrokeTool toolType;

	// The array with the points
	Point* points;
	int pointCount;
	int pointAllocCount;

	// Stroke timestamp, to match it to the audio stream
	int timestamp;
	string audioFilename;

	EraseableStroke* eraseable;
};
