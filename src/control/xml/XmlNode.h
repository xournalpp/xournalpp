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

#include "Attribute.h"

#include <OutputStream.h>
#include <Util.h>

class ProgressListener;

class XmlNode
{
public:
	XmlNode(const char* tag);
	virtual ~XmlNode();

private:
	XmlNode(const XmlNode& node);
	void operator=(const XmlNode& node);

public:
	void setAttrib(const char* attrib, string value);
	void setAttrib(const char* attrib, const char* value);
	void setAttrib(const char* attrib, double value);
	void setAttrib(const char* attrib, int value);
	void setAttrib(const char* attrib, size_t value);

	/**
	 * The double array is now owned by XmlNode and automatically deleted!
	 */
	void setAttrib(const char* attrib, double* value, int count);

	void writeOut(OutputStream* out, ProgressListener* _listener);

	virtual void writeOut(OutputStream* out)
	{
		writeOut(out, nullptr);
	}

	void addChild(XmlNode* node);

protected:
	void putAttrib(XMLAttribute* a);
	void writeAttributes(OutputStream* out);

public:
	protected:
	GList* children;
	GList* attributes;

	char* tag;
};
