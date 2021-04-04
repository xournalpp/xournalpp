#include "LineBackgroundPainter.h"

#include "Util.h"

LineBackgroundPainter::LineBackgroundPainter(bool verticalLine): verticalLine(verticalLine) {}

LineBackgroundPainter::~LineBackgroundPainter() = default;

void LineBackgroundPainter::resetConfig() {
    this->defaultForegroundColor1 = 0x40A0FFU;
    this->defaultAlternativeForegroundColor1 = 0x434343U;
    this->defaultForegroundColor2 = 0xFF0080U;
    this->defaultAlternativeForegroundColor2 = 0x220080U;
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

const double rulingSize = 24;

void LineBackgroundPainter::paintBackgroundRuled() {
    Util::cairo_set_source_rgbi(cr, this->foregroundColor1);
    cairo_set_line_width(cr, lineWidth * lineWidthFactor);

    int numLines = static_cast<int>((height - headerSize - footerSize) / (rulingSize + lineWidth * lineWidthFactor));

    double offset = headerSize;

    for (int i = 0; i < numLines; i++) {
        cairo_move_to(cr, 0, offset);
        cairo_line_to(cr, width, offset);
        offset += rulingSize;
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
