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

#ifndef __XMLIMAGENODE_H__
#define __XMLIMAGENODE_H__

#include "XmlNode.h"

class XmlImageNode: public XmlNode {
public:
	XmlImageNode(const char * tag);
	~XmlImageNode();

public:
	void setImage(cairo_surface_t * img);

	static cairo_status_t pngWriteFunction(XmlImageNode * image, unsigned char *data, unsigned int length);

	virtual void writeOut(OutputStream * out);

private:
	cairo_surface_t * img;

	OutputStream * out;
	int pos;
	unsigned char buffer[30];
};

#endif /* __XMLIMAGENODE_H__ */
