#include "PageOrientationHelper.h"

void xoj::view::cairoRotatePage(cairo_t* cr, PageOrientation pageOrient, double pageWidth, double pageHeight) {
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
}
