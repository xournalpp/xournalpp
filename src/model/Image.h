/*
 * Xournal++
 *
 * An Image on the document
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "Element.h"

class Image: public Element {
public:
	Image();
	virtual ~Image();

	void setWidth(double width);
	void setHeight(double height);

	void setImage(unsigned char * data, int len);
	cairo_surface_t * getImage();

private:
	virtual void calcSize();

	static cairo_status_t cairoReadFunction(Image * image, unsigned char *data, unsigned int length);
private:

	cairo_surface_t * image;

	unsigned char * data;
	int dLen;

	int read;
};

#endif /* __IMAGE_H__ */
