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

#include "pdf/base/XojPdfPage.h"
#include <XournalType.h>

#include <cairo/cairo.h>
#include <list>

class PdfCacheEntry;
typedef std::list<PdfCacheEntry*> PdfCacheEntryList;

class PdfCache
{
public:
	PdfCache(int size);
	virtual ~PdfCache();

private:
	PdfCache(const PdfCache& cache);
	void operator=(const PdfCache& cache);

public:
	void render(cairo_t* cr, XojPdfPageSPtr popplerPage, double zoom);

private:
	void setZoom(double zoom);
	void clearCache();
	cairo_surface_t* lookup(XojPdfPageSPtr popplerPage);
	void cache(XojPdfPageSPtr popplerPage, cairo_surface_t* img);

private:
	XOJ_TYPE_ATTRIB;

	GMutex renderMutex;

	PdfCacheEntryList data;
	PdfCacheEntryList::size_type size;

	double zoom;
};
