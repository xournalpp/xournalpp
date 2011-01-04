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

#include "../model/Page.h"
#include <poppler.h>

class SearchControl {
public:
	SearchControl(XojPage * page, PopplerPage * pdf);
	virtual ~SearchControl();

	bool search(const char * text, int * occures, double * top);
	void paint(cairo_t * cr, GdkEventExpose *event, double zoom, GdkColor color);
private:
	void freeSearchResults();

private:
	XojPage * page;
	PopplerPage * pdf;

	GList * results;
};

#endif /* __SEARCHCONTROL_H__ */
