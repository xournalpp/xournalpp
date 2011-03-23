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
// TODO: AA: type check

#ifndef __SEARCHCONTROL_H__
#define __SEARCHCONTROL_H__

#include "../model/Page.h"
#include "../pdf/poppler/XojPopplerPage.h"

class SearchControl {
public:
	SearchControl(XojPage * page, XojPopplerPage * pdf);
	virtual ~SearchControl();

	bool search(const char * text, int * occures, double * top);
	void paint(cairo_t * cr, GdkRectangle * rect, double zoom, GdkColor color);
private:
	void freeSearchResults();

private:
	XojPage * page;
	XojPopplerPage * pdf;

	GList * results;
};

#endif /* __SEARCHCONTROL_H__ */
