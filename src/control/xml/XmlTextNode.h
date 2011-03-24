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

#ifndef __XMLTEXTNODE_H__
#define __XMLTEXTNODE_H__

#include "XmlNode.h"

class XmlTextNode: public XmlNode {
public:
	XmlTextNode(const char * tag, const char * text);
	XmlTextNode(const char * tag);
	~XmlTextNode();

public:
	void setText(const char * text);

	virtual void writeOut(OutputStream * out);

private:
	XOJ_TYPE_ATTRIB;

	char * text;
};

#endif /* __XMLTEXTNODE_H__ */
