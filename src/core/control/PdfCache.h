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

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include <cairo.h>
#include <glib.h>

#include "pdf/base/XojPdfPage.h"


class PdfCacheEntry;

class PdfCache {
public:
    PdfCache(int size);
    virtual ~PdfCache();

private:
    PdfCache(const PdfCache& cache);
    void operator=(const PdfCache& cache);

public:
    void render(cairo_t* cr, const XojPdfPageSPtr& popplerPage, double zoom);
    void clearCache();

public:
    /**
     * @param b true iff any change in the view's zoom as compared to when a page
     *  was cached forces a re-render.
     */
    void setAnyZoomChangeCausesRecache(bool b);

    /**
     * @brief Set the maximum tolerable zoom difference, as a percentage.
     *
     * @param threshold is the minimum percent-difference between the zoom value at
     *  which the cached version of the page was rendered and the current zoom,
     *  for which the page will be re-cached while zooming.
     */
    void setRefreshThreshold(double percentDifference);

private:
    void setZoom(double zoom);
    PdfCacheEntry* lookup(const XojPdfPageSPtr& popplerPage);
    PdfCacheEntry* cache(XojPdfPageSPtr popplerPage, cairo_surface_t* img, double zoom);

private:
    std::mutex renderMutex;

    std::list<PdfCacheEntry*> data;
    std::list<PdfCacheEntry*>::size_type size = 0;

    double zoom = -1;
    double zoomRefreshThreshold;
    bool zoomClearsCache = true;
};
