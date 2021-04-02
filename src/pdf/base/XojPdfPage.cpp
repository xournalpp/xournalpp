#include "XojPdfPage.h"

XojPdfRectangle::XojPdfRectangle() = default;

XojPdfRectangle::XojPdfRectangle(const Rectangle<double>& rect):
        x1{rect.x}, y1{rect.y}, x2{x1 + rect.width}, y2{y1 + rect.height} {}

XojPdfRectangle::XojPdfRectangle(double x1, double y1, double x2, double y2): x1(x1), y1(y1), x2(x2), y2(y2) {}

XojPdfPage::XojPdfPage() = default;

XojPdfPage::~XojPdfPage() = default;

void XojPdfPage::render(cairo_t* cr, XojPdfRectangle region) {
    cairo_save(cr);
    cairo_rectangle(cr, region.x1, region.y1, region.x2, region.y2);
    cairo_clip(cr);

    this->render(cr, false);

    cairo_restore(cr);
}
