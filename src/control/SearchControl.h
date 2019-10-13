/*
 * Xournal++
 *
 * Handles text search on a PDF page and in Xournal Texts
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "model/PageRef.h"
#include "util/GtkColorWrapper.h"
#include "pdf/base/XojPdfPage.h"

class SearchControl
{
public:
	SearchControl(PageRef page, XojPdfPageSPtr pdf);
	virtual ~SearchControl();

	bool search(string text, int* occures, double* top);
	void paint(cairo_t* cr, GdkRectangle* rect, double zoom, GtkColorWrapper color);
private:
	void freeSearchResults();

private:
	PageRef page;
	XojPdfPageSPtr pdf;

	vector<XojPdfRectangle> results;
};
