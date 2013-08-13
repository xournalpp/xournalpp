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

#ifndef __XMLTEXNODE_H__
#define __XMLTEXNODE_H__

#include "XmlNode.h"

class XmlTexNode: public XmlNode {
public:
	XmlTexNode(const char * tag);
	virtual ~XmlTexNode();

public:
	void setImage(cairo_surface_t * img);

	static cairo_status_t pngWriteFunction(XmlTexNode * image, unsigned char *data, unsigned int length);

	virtual void writeOut(OutputStream * out);

private:
	XOJ_TYPE_ATTRIB;

	cairo_surface_t * img;

	OutputStream * out;
	int pos;
	unsigned char buffer[30];
};

#endif /* __XMLTEXNODE_H__ */
