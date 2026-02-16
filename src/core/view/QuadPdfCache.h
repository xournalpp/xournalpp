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
#include <mutex>    // for mutex
#include <vector>

#include <cairo.h>  // for cairo_t, cairo_surface_t

#include "pdf/base/XojPdfDocument.h"  // for XojPdfDocument

#include "PdfCache.h"
#include "QuadCache.h"  // for QuadCache::TileInfo

class Settings;

namespace xoj::view {
class QuadPdfCache: public PdfCache {
public:
    QuadPdfCache(const XojPdfDocument& doc, Settings* settings);
    ~QuadPdfCache() override;

    QuadPdfCache(const QuadPdfCache& cache) = delete;
    void operator=(const QuadPdfCache& cache) = delete;

    struct Entry;

public:
    /**
     * @brief Paint the page with number pdfPageNo of the pdf document to the cairo context
     * @param cr the cairo context
     * @param pdfPageNo The page number (in the pdf document)
     * @param pageWidth/pageHeight Xournal++ page dimensions
     */
    void paint(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight) override;

    /**
     * @brief Paint the part of the PDF page pdfPageNo corresponding to the TileInfo
     * @param cr the cairo context
     * @param pdfPageNo The page number (in the pdf document)
     * @param ti The tile to paint
     * @param pageWidth/pageHeight Xournal++ page dimensions
     * Falls back to a normal paint if the Xournal++ page and the PDF page do not have the same dimensions
     */
    void paintSingleTile(cairo_t* cr, size_t pdfPageNo, const xoj::view::QuadCache::TileInfo& ti, double pageWidth,
                         double pageHeight);

    /// Clears the oldest buffers to reduce the size in memory below maxMemoryUsageInMB
    void prune();

public:
    /// in MB
    void setMaxMemoryUsage(unsigned int usage);

private:
    /**
     * @brief Look up for a cache entry for the page with number pdfPgeNo in the PDF.
     * Creates one if none exist. Returns nullptr if no PDF page correspond to the given number
     */
    Entry* lookup(size_t pdfPageNo);

private:
    XojPdfDocument pdfDocument;

    std::mutex renderMutex;
    std::vector<std::unique_ptr<Entry>> data;
    unsigned int maxMemoryUsageInMB;
};
}  // namespace xoj::view
