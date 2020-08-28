#include "LineBackgroundPainter.h"

#include "Util.h"

LineBackgroundPainter::LineBackgroundPainter(bool verticalLine): verticalLine(verticalLine) {}

LineBackgroundPainter::~LineBackgroundPainter() = default;

void LineBackgroundPainter::resetConfig() {
    this->foregroundColor1 = 0x40A0FFU;
    this->foregroundColor2 = 0xFF0080U;
    this->lineWidth = 0.5;
}

void LineBackgroundPainter::paint() {
    paintBackgroundColor();

    paintBackgroundRuled();

    if (verticalLine) {
        paintBackgroundVerticalLine();
    }
}

const double headerSize = 80;
const double footerSize = 20;

const double roulingSize = 24;

void LineBackgroundPainter::paintBackgroundRuled() {
    Util::cairo_set_source_rgbi(cr, this->foregroundColor1);
    cairo_set_line_width(cr, lineWidth * lineWidthFactor);

    int numLines = static_cast<int>((height - headerSize - footerSize) / (roulingSize + lineWidth * lineWidthFactor));

    double offset = headerSize;

    for (int i = 0; i < numLines; i++) {
        cairo_move_to(cr, 0, offset);
        cairo_line_to(cr, width, offset);
        offset += roulingSize;
    }

    cairo_stroke(cr);
}

void LineBackgroundPainter::paintBackgroundVerticalLine() {
    Util::cairo_set_source_rgbi(cr, this->foregroundColor2);
    cairo_set_line_width(cr, lineWidth * lineWidthFactor);

    cairo_move_to(cr, 72, 0);
    cairo_line_to(cr, 72, height);
    cairo_stroke(cr);
}
