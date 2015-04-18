/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2
 */

#pragma once

#include "XmlNode.h"
#include "../../model/Point.h"

class XmlPointNode : public XmlNode
{
public:
	XmlPointNode(const char* tag);
	virtual ~XmlPointNode();

private:
	XmlPointNode(const XmlPointNode& node);
	void operator=(const XmlPointNode& node);

public:
	void addPoint(const Point* point);

	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	GList* points;
};
