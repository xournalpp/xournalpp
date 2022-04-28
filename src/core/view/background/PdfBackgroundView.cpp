#include "PdfBackgroundView.h"

#include "control/PdfCache.h"

using namespace xoj::view;

PdfBackgroundView::PdfBackgroundView(double pageWidth, double pageHeight, size_t pageNo, PdfCache* pdfCache):
        BackgroundView(pageWidth, pageHeight), pageNo(pageNo), pdfCache(pdfCache) {}

void PdfBackgroundView::draw(cairo_t* cr) const {
    if (pdfCache) {
        // get zoom from cairo
        cairo_matrix_t matrix = {0};
        cairo_get_matrix(cr, &matrix);
        assert(matrix.xx == matrix.yy && matrix.xy == 0.0 && matrix.yx == 0.0);  // Homothety matrix
        pdfCache->render(cr, pageNo, matrix.xx, pageWidth, pageHeight);
    } else {
        g_warning("PdfBackgroundView::draw Missing pdf cache: cannot render the pdf page");
        PdfCache::renderMissingPdfPage(cr, pageWidth, pageHeight);
    }
}
