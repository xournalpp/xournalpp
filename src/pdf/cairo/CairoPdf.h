/*
 * Xournal++
 *
 * Used to output cairo contents to PDF
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __CAIROPDF_H__
#define __CAIROPDF_H__

#include <cairo.h>
#include "../poppler/XojPopplerPage.h"
#include "../poppler/XojPopplerDocument.h"
#include "../../model/Page.h"

class CairoPdf {
public:
	CairoPdf();
	virtual ~CairoPdf();

public:
	void drawPage(XojPage * page);
	XojPopplerPage * getPage(int page);
	XojPopplerDocument & getDocument();

	void finalize();

private:
	static cairo_status_t writeOut(CairoPdf * pdf, unsigned char * data, unsigned int length);

private:
	GString * data;

	XojPopplerDocument doc;

	cairo_surface_t * surface;
	cairo_t * cr;
};

#endif /* __CAIROPDF_H__ */
