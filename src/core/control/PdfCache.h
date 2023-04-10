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

#include <cstddef>  // for size_t
#include <deque>    // for deque
#include <mutex>    // for mutex

#include <cairo.h>  // for cairo_t, cairo_surface_t

#include "pdf/base/XojPdfDocument.h"  // for XojPdfDocument
#include "pdf/base/XojPdfPage.h"      // for XojPdfPageSPtr

namespace xoj::view {
class Mask;
};

class PdfCacheEntry;
class Settings;

class PdfCache {
public:
    PdfCache(const XojPdfDocument& doc, Settings* settings);
    virtual ~PdfCache();

private:
    PdfCache(const PdfCache& cache);
    void operator=(const PdfCache& cache);

public:
    /**
     * @brief Render the page with number pdfPageNo of the pdf document to the cairo context
     * @param cr the cairo context
     * @param pdfPageNo The page number (in the pdf document)
     * @param zoom The current zoom level
     * @param pageWidth/pageHeight Xournal++ page dimensions
     */
    void render(cairo_t* cr, size_t pdfPageNo, double zoom, double pageWidth, double pageHeight);

public:
    /**
     * @brief Set the maximum tolerable zoom difference, as a percentage.
     *
     * @param threshold is the minimum percent-difference between the zoom value at
     *  which the cached version of the page was rendered and the current zoom,
     *  for which the page will be re-cached while zooming.
     */
    void setRefreshThreshold(double percentDifference);

    void setMaxSize(size_t newSize);

    void updateSettings(Settings* settings);

    /**
     * @brief Renders an error background, for when the pdf page cannot be rendered
     */
    static void renderMissingPdfPage(cairo_t* cr, double pageWidth, double pageHeight);

private:
    /**
     * @brief Look up for a cache entry for the page with number pdfPgeNo in the PDF.
     */
    const PdfCacheEntry* lookup(size_t pdfPageNo) const;
    /**
     * @brief Push a cache entry
     */
    const PdfCacheEntry* cache(XojPdfPageSPtr popplerPage, xoj::view::Mask&& buffer);

private:
    XojPdfDocument pdfDocument;

    std::mutex renderMutex;

    std::deque<std::unique_ptr<PdfCacheEntry>> data;
    decltype(data)::size_type maxSize = 0;

    double zoomRefreshThreshold;
};
