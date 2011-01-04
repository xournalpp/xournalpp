/*
 * Xournal++
 *
 * Paints a page to a cairo context
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */

#ifndef __DOCUMENTVIEW_H__
#define __DOCUMENTVIEW_H__
#include <poppler-page.h>

#include <gtk/gtk.h>
#include "../model/Element.h"
#include "../model/Stroke.h"
#include "../model/Text.h"
#include "../model/Page.h"

class DocumentView {
public:
	DocumentView();
	virtual ~DocumentView();

public:
	void drawPage(XojPage * page, PopplerPage * popplerPage, cairo_t *cr);
	void drawStroke(cairo_t *cr, Stroke * s, int startPoint = 0);
	static void applyColor(cairo_t *cr, int c, int alpha = 255);
	static void applyColor(cairo_t *cr, Element * e, int alpha = 255);

private:
	void drawText(cairo_t *cr, Text * t);
	void drawLayer(cairo_t *cr, Layer * l);

	void paintBackgroundColor();
	void paintBackgroundGraph();
	void paintBackgroundLined();
	void paintBackgroundRuled();

private:
	cairo_t * cr;
	XojPage * page;
	double width;
	double height;
};

#endif /* __DOCUMENTVIEW_H__ */
