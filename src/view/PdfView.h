/*
 * Xournal++
 *
 * Paints a pdf Page to a cairo context, handles special cases
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __PDFVIEW_H__
#define __PDFVIEW_H__

#include "../pdf/popplerdirect/poppler/XojPopplerPage.h"
#include <cairo/cairo.h>
#include "../control/PdfCache.h"

class PdfView {
private:
	PdfView();
	virtual ~PdfView();

public:
	static void drawPage(PdfCache * cache, XojPopplerPage * popplerPage, cairo_t * cr, double zoom, double width, double height, bool forPrinting = false);
};

#endif /* __PDFVIEW_H__ */
