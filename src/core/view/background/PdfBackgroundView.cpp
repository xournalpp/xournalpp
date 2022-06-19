#include "PdfBackgroundView.h"

#include <cassert>  // for assert

#include <glib.h>  // for g_warning

#include "control/PdfCache.h"                // for PdfCache
#include "view/background/BackgroundView.h"  // for BackgroundView, view

using namespace xoj::view;

PdfBackgroundView::PdfBackgroundView(double pageWidth, double pageHeight, size_t pageNo, PdfCache* pdfCache):
        BackgroundView(pageWidth, pageHeight), pageNo(pageNo), pdfCache(pdfCache) {}

void PdfBackgroundView::draw(cairo_t* cr) const {
    if (pdfCache) {
        // get zoom from cairo
        cairo_matrix_t matrix = {0};
        cairo_get_matrix(cr, &matrix);
        assert(matrix.xx == matrix.yy && matrix.xy == 0.0 && matrix.yx == 0.0);  // Homothety matrix
        double scaleX;
        double scaleY;
        cairo_surface_get_device_scale(cairo_get_target(cr), &scaleX, &scaleY);
        assert(scaleX == scaleY);
        double pixelsPerPageUnit = matrix.xx * scaleX;
        pdfCache->render(cr, pageNo, pixelsPerPageUnit, pageWidth, pageHeight);
    } else {
        g_warning("PdfBackgroundView::draw Missing pdf cache: cannot render the pdf page");
        PdfCache::renderMissingPdfPage(cr, pageWidth, pageHeight);
    }
}
