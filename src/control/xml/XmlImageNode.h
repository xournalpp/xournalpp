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

class XmlImageNode : public XmlNode
{
public:
	XmlImageNode(const char* tag);
	virtual ~XmlImageNode();

public:
	void setImage(cairo_surface_t* img);

	static cairo_status_t pngWriteFunction(XmlImageNode* image, unsigned char* data, unsigned int length);

	virtual void writeOut(OutputStream* out);

private:
	cairo_surface_t* img;

	OutputStream* out;
	int pos;
	unsigned char buffer[30] = { 0 };
};
