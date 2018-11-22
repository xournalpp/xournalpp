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

	int getTimestamp();
	void setTimestamp(int seconds);
	string getAudioFilename();
	void setAudioFilename(string filename);
	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	GList* points;
	int timestamp;
	string audioFilename;
};
