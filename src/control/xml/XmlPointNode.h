/*
 * Xournal++
 *
 * XML Writer helper class
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __XMLPOINTNODE_H__
#define __XMLPOINTNODE_H__

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

#endif /* __XMLPOINTNODE_H__ */
