#include "DottedBackgroundPainter.h"

#include "Util.h"

DottedBackgroundPainter::DottedBackgroundPainter() = default;

DottedBackgroundPainter::~DottedBackgroundPainter() = default;

void DottedBackgroundPainter::resetConfig() {
    this->foregroundColor1 = 0xBDBDBDU;
    this->lineWidth = 1.5;
    this->drawRaster1 = 14.17;
}

void DottedBackgroundPainter::paint() {
    paintBackgroundColor();
    paintBackgroundDotted();
}

void DottedBackgroundPainter::paintBackgroundDotted() {
    Util::cairo_set_source_rgbi(cr, this->foregroundColor1);

    cairo_set_line_width(cr, lineWidth * lineWidthFactor);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    auto pos = [dr1 = drawRaster1](int i) { return dr1 + i * dr1; };
    for (int x = 0; pos(x) < width; ++x) {
        for (int y = 0; pos(y) < height; ++y) {
            cairo_move_to(cr, pos(x), pos(y));
            cairo_line_to(cr, pos(x), pos(y));
        }
    }

    cairo_stroke(cr);
}
