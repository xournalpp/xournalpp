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

#ifndef __XMLNODE_H__
#define __XMLNODE_H__

#include "../../util/Util.h"
#include "../../util/OutputStream.h"
#include "Attribute.h"
#include <glib.h>

class XmlNode {
public:
	XmlNode(const char * tag);
	~XmlNode();

public:
	void setAttrib(const char * attrib, const char * value);
	void setAttrib(const char * attrib, double value);
	void setAttrib(const char * attrib, int value);

	/**
	 * The double array is now owned by XmlNode and automatically deleted!
	 */
	void setAttrib(const char * attrib, double * value, int count);

	virtual void writeOut(OutputStream * out);
	void addChild(XmlNode * node);

protected:
	void putAttrib(Attribute * a);
	void writeAttributes(OutputStream * out);

public:
	XOJ_TYPE_ATTRIB;

protected:
	GList * children;
	GList * attributes;

	char * tag;
};

#endif /* __XMLNODE_H__ */
