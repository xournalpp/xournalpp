/*
 * Xournal++
 *
 * A TexImage on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "Element.h"
#include <XournalType.h>

#include <string>
using std::string;

class TexImage: public Element
{
public:
	TexImage();
	virtual ~TexImage();

public:
	void setWidth(double width);
	void setHeight(double height);

	void setImage(string data);
	void setImage(cairo_surface_t* image);
	void setImage(GdkPixbuf* img);
	cairo_surface_t* getImage();

	virtual void scale(double x0, double y0, double fx, double fy);
	virtual void rotate(double x0, double y0, double xo, double yo, double th);

	//text tag to alow latex
	void setText(string text);
	string getText();

	virtual Element* clone();

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in) throw (InputStreamException);

private:
	virtual void calcSize();

	static cairo_status_t cairoReadFunction(TexImage* image, unsigned char* data, unsigned int length);
private:
	XOJ_TYPE_ATTRIB;


	cairo_surface_t* image;

	string data;

	string::size_type read;
	//text
	string text;
};
