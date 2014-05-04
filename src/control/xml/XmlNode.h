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

#ifndef __XMLNODE_H__
#define __XMLNODE_H__

#include <Util.h>
#include <OutputStream.h>
#include "Attribute.h"
#include <glib.h>

class ProgressListener;

class XmlNode
{
public:
	XmlNode(const char* tag);
	virtual ~XmlNode();

private:
	XmlNode(const XmlNode& node);
	void operator =(const XmlNode& node);

public:
	void setAttrib(const char* attrib, const char* value);
	void setAttrib(const char* attrib, double value);
	void setAttrib(const char* attrib, int value);

	/**
	 * The double array is now owned by XmlNode and automatically deleted!
	 */
	void setAttrib(const char* attrib, double* value, int count);

	virtual void writeOut(OutputStream* out,
	                      ProgressListener* listener = NULL);
	void addChild(XmlNode* node);

protected:
	void putAttrib(Attribute* a);
	void writeAttributes(OutputStream* out);

public:
	XOJ_TYPE_ATTRIB;

protected:
	GList* children;
	GList* attributes;

	char* tag;
};

#endif /* __XMLNODE_H__ */
