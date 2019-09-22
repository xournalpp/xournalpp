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
#include "XmlAudioNode.h"

class XmlTextNode : public XmlAudioNode
{
public:
	XmlTextNode(const char* tag, const char* text);
	XmlTextNode(const char* tag);
	virtual ~XmlTextNode();

public:
	void setText(const char* text);

	virtual void writeOut(OutputStream* out);

private:
	char* text;
};
