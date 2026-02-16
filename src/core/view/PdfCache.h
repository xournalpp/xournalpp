/*
 * Xournal++
 *
 * Interface for Caching PDF backgrounds for faster repaint
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstddef>  // for size_t

#include <cairo.h>  // for cairo_t, cairo_surface_t

class Settings;

namespace xoj::view {
class PdfCache {
public:
    PdfCache() = default;
    virtual ~PdfCache() = default;

    PdfCache(const PdfCache& cache) = delete;
    void operator=(const PdfCache& cache) = delete;

public:
    /**
     * @brief Paint the page with number pdfPageNo of the pdf document to the cairo context
     * @param cr the cairo context
     * @param pdfPageNo The page number (in the pdf document)
     * @param pageWidth/pageHeight Xournal++ page dimensions
     */
    virtual void paint(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight) = 0;

    /// @brief Renders an error background, for when the pdf page cannot be rendered
    static void renderMissingPdfPage(cairo_t* cr, size_t pdfPageNo, double pageWidth, double pageHeight);
};
}  // namespace xoj::view
