#include "PdfView.h"

#include <config.h>

#include "util/i18n.h"

PdfView::PdfView() = default;

PdfView::~PdfView() = default;

void PdfView::drawPage(PdfCache* cache, size_t pageNo, cairo_t* cr, double zoom, double width, double height) {
    if (cache) {
        cairo_set_source_rgb(cr, 1., 1., 1.);
        cairo_paint(cr);
        cache->render(cr, pageNo, zoom, width, height);
    } else {
        PdfCache::renderMissingPdfPage(cr, width, height);
    }
}
