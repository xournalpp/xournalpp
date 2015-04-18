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

class XmlTextNode : public XmlNode
{
public:
	XmlTextNode(const char* tag, const char* text);
	XmlTextNode(const char* tag);
	virtual ~XmlTextNode();

public:
	void setText(const char* text);

	virtual void writeOut(OutputStream* out);

private:
	XOJ_TYPE_ATTRIB;

	char* text;
};
