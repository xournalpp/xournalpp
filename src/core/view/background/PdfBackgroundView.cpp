#include "PdfBackgroundView.h"

#include <cmath>  // for M_PI_2

#include <glib.h>  // for g_warning

#include "control/PdfCache.h"                // for PdfCache
#include "util/Assert.h"                     // for xoj_assert
#include "view/background/BackgroundView.h"  // for BackgroundView, view

using namespace xoj::view;

PdfBackgroundView::PdfBackgroundView(double pageWidth, double pageHeight, size_t pageNo, PdfCache* pdfCache,
                                     PageOrientation pageOrient):
        BackgroundView(pageWidth, pageHeight), pageNo(pageNo), pdfCache(pdfCache), pageOrient(pageOrient) {}

void PdfBackgroundView::draw(cairo_t* cr) const {
    if (pdfCache) {
        // get zoom from cairo
        cairo_matrix_t matrix = {0};
        cairo_get_matrix(cr, &matrix);
        xoj_assert(matrix.xx == matrix.yy && matrix.xy == 0.0 && matrix.yx == 0.0);  // Homothety matrix
        double scaleX;
        double scaleY;
        cairo_surface_get_device_scale(cairo_get_target(cr), &scaleX, &scaleY);
        xoj_assert(scaleX == scaleY);
        double pixelsPerPageUnit = matrix.xx * scaleX;

        // rotate page if necessary
        if (pageOrient != PageOrientation::UP) {
            cairo_save(cr);
            cairo_translate(cr, pageWidth / 2, pageHeight / 2);
            cairo_rotate(cr, static_cast<int>(pageOrient) * M_PI_2);
            if (pageOrient == PageOrientation::DOWN) {
                cairo_translate(cr, -pageWidth / 2, -pageHeight / 2);
            } else {
                cairo_translate(cr, -pageHeight / 2, -pageWidth / 2);
            }
        }

        pdfCache->render(cr, pageNo, pixelsPerPageUnit, pageWidth, pageHeight);

        if (pageOrient != PageOrientation::UP) {
            cairo_restore(cr);
        }
    } else {
        g_warning("PdfBackgroundView::draw Missing pdf cache: cannot render the pdf page");
        PdfCache::renderMissingPdfPage(cr, pageWidth, pageHeight);
    }
}
