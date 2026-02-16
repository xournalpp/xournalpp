/*
 * Xournal++
 *
 * Caches PDF backgrounds for faster repaint - Optimized for when the zoom rarely changes
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t
#include <mutex>    // for mutex
#include <vector>

#include <cairo.h>  // for cairo_t, cairo_surface_t

#include "pdf/base/XojPdfDocument.h"  // for XojPdfDocument
#include "pdf/base/XojPdfPage.h"      // for XojPdfPageSPtr

#include "PdfCache.h"

namespace xoj::view {
class Mask;

class FlatPdfCache: public PdfCache {
public:
    FlatPdfCache(const XojPdfDocument& doc);
    ~FlatPdfCache() override;

    class Entry;

    FlatPdfCache(const FlatPdfCache& cache) = delete;
    void operator=(const FlatPdfCache& cache) = delete;

public:
    /**
     * @brief Paint the page with number pdfPageNo of the pdf document to the cairo context
     * @param cr the cairo context
     * @param pdfPageNo The page number (in the pdf document)
     * @param zoom The current zoom level
     * @param pageWidth/pageHeight Xournal++ page dimensions
     */
    void paint(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight) override;

private:
    /// Adds an entry - removes some other entries if the cache is too big
    Entry* cache(XojPdfPageSPtr popplerPage, Mask&& buffer);
    /// @brief Look up for a cache entry for the page with number pdfPgeNo in the PDF.
    Entry* lookup(size_t pdfPageNo) const;

private:
    XojPdfDocument pdfDocument;

    std::mutex renderMutex;
    std::vector<std::unique_ptr<Entry>> entries;
    size_t sizeInkB;
};
}  // namespace xoj::view
