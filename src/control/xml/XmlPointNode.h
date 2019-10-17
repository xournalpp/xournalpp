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

#include "model/Point.h"
#include "XmlAudioNode.h"

class XmlPointNode : public XmlAudioNode
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
	GList* points;
};
