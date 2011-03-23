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
// TODO: AA: type check

#ifndef __XMLPOINTNODE_H__
#define __XMLPOINTNODE_H__

#include "XmlNode.h"
#include "../../model/Point.h"

class XmlPointNode: public XmlNode {
public:
	XmlPointNode(const char * tag);
	~XmlPointNode();

public:
	/**
	 * The point array is owned by the XML Node and automatically deleted
	 */
	void setPoints(Point * points, int count);

	virtual void writeOut(OutputStream * out);

private:
	Point * points;
	int count;
};

#endif /* __XMLPOINTNODE_H__ */
