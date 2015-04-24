/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "XmlNode.h"
#include "model/Point.h"

class XmlStrokeNode : public XmlNode
{
public:
	XmlStrokeNode(const char* tag);
	virtual ~XmlStrokeNode();

public:
	void setPoints(Point* points, int pointLength);
	void setWidth(double width, double* widths, int widthsLength);

	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	Point* points;
	int pointLength;

	double width;

	double* widths;
	int widthsLength;
};
