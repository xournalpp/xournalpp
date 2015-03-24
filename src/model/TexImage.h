/*
 * Xournal++
 *
 * A TexImage on the document
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv3
 */

#ifndef __TEXIMAGE_H__
#define __TEXIMAGE_H__

#include "Element.h"
#include <XournalType.h>

class TexImage: public Element
{
public:
	TexImage();
	virtual ~TexImage();

public:
	void setWidth(double width);
	void setHeight(double height);

	void setImage(unsigned char* data, int len);
	void setImage(cairo_surface_t* image);
	void setImage(GdkPixbuf* img);
	cairo_surface_t* getImage();

	virtual void scale(double x0, double y0, double fx, double fy);

	//text tag to alow latex
	void setText(const char* text, int textlength);
	const char* getText();
	int getTextLen();

	virtual Element* clone();

public:
	// Serialize interface
	void serialize(ObjectOutputStream& out);
	void readSerialized(ObjectInputStream& in) throw (InputStreamException);

private:
	virtual void calcSize();

	static cairo_status_t cairoReadFunction(TexImage* image, unsigned char* data,
	                                        unsigned int length);
private:
	XOJ_TYPE_ATTRIB;


	cairo_surface_t* image;

	unsigned char* data;
	int dLen;

	int read;
	//text
	const char* text;
	int textlen;
};

#endif /* __TEXIMAGE_H__ */
