/*
 * Xournal++
 *
 * Handles text search on a PDF page and in Xournal Texts
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __SEARCHCONTROL_H__
#define __SEARCHCONTROL_H__

#include "../model/PageRef.h"
#include "../pdf/popplerdirect/poppler/XojPopplerPage.h"

class SearchControl {
public:
	SearchControl(PageRef page, XojPopplerPage * pdf);
	virtual ~SearchControl();

	bool search(const char * text, int * occures, double * top);
	void paint(cairo_t * cr, GdkRectangle * rect, double zoom, GdkColor color);
private:
	void freeSearchResults();

private:
	XOJ_TYPE_ATTRIB;

	PageRef page;
	XojPopplerPage * pdf;

	GList * results;
};

#endif /* __SEARCHCONTROL_H__ */
