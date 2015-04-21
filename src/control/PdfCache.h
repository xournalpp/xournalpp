/*
 * Xournal++
 *
 * Caches PDF backgrounds for faster repaint
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo/cairo.h>
#include "../pdf/popplerdirect/poppler/XojPopplerPage.h"
#include <XournalType.h>

class PdfCache
{
public:
	PdfCache(int size);
	virtual ~PdfCache();

private:
	PdfCache(const PdfCache& cache);
	void operator=(const PdfCache& cache);

public:
	void render(cairo_t* cr, XojPopplerPage* popplerPage, double zoom);

private:
	void setZoom(double zoom);
	void clearCache();
	cairo_surface_t* lookup(XojPopplerPage* popplerPage);
	void cache(XojPopplerPage* popplerPage, cairo_surface_t* img);

private:
	XOJ_TYPE_ATTRIB;

	GMutex renderMutex;

	GList* data;
	int size;

	double zoom;
};
