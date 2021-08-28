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

#include "pdf/base/XojPdfPage.h"

#include "QuadTreeCache.h"

class PdfCache {
public:
    PdfCache(size_t maxSize_px);
    ~PdfCache();

    /**
     * Renders and caches a region of a page.
     */
    void render(cairo_t* cr, const XojPdfPageSPtr& popplerPage, Rectangle<double> srcRegion, double zoom);

    void clear();

private:
    class CacheEntry;
    std::shared_ptr<CacheEntry> lookup(XojPdfPageSPtr popplerPage);

private:
    std::mutex renderMutex_{};
    QuadTreeCache::CacheParams cacheSettings_;
    std::list<std::shared_ptr<CacheEntry> > data_;
};
