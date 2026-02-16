#include "PdfBackgroundView.h"

#include <glib.h>  // for g_warning

#include "util/Assert.h"                     // for xoj_assert
#include "view/PdfCache.h"                   // for PdfCache
#include "view/background/BackgroundView.h"  // for BackgroundView, view

using namespace xoj::view;

PdfBackgroundView::PdfBackgroundView(double pageWidth, double pageHeight, size_t pageNo, PdfCache* pdfCache):
        BackgroundView(pageWidth, pageHeight), pageNo(pageNo), pdfCache(pdfCache) {}

void PdfBackgroundView::draw(cairo_t* cr) const {
    if (pdfCache) {
        pdfCache->paint(cr, pageNo, pageWidth, pageHeight);
    } else {
        PdfCache::renderMissingPdfPage(cr, pageNo, pageWidth, pageHeight);
    }
}
