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
#include "../popplerdirect/poppler/XojPopplerPage.h"
#include "../popplerdirect/poppler/XojPopplerDocument.h"
#include "../../model/PageRef.h"
#include "../../util/XournalType.h"

class CairoPdf {
public:
	CairoPdf();
	virtual ~CairoPdf();

public:
	void drawPage(PageRef page);
	XojPopplerPage * getPage(int page);
	XojPopplerDocument & getDocument();

	void finalize();

private:
	static cairo_status_t writeOut(CairoPdf * pdf, unsigned char * data, unsigned int length);

private:
	XOJ_TYPE_ATTRIB;


	GString * data;

	XojPopplerDocument doc;

	cairo_surface_t * surface;
	cairo_t * cr;
};

#endif /* __CAIROPDF_H__ */
