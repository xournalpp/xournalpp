#include "StavesBackgroundPainter.h"

#include "Util.h"

StavesBackgroundPainter::StavesBackgroundPainter() = default;

StavesBackgroundPainter::~StavesBackgroundPainter() = default;

void StavesBackgroundPainter::resetConfig() {
    this->foregroundColor1 = 0x000000U;
    this->foregroundColor2 = 0xFF0080U;
    this->lineWidth = 0.5;
}

void StavesBackgroundPainter::paint() {
    paintBackgroundColor();

    double lineSize = 4 * staveDistance + 5 * lineWidth * lineWidthFactor + lineDistance;
    double offset = headerSize;

    int numStaves = static_cast<int>((height - headerSize - footerSize + lineDistance) / (lineSize));

    for (int line = 0; line < numStaves; line++) {
        paintBackgroundStaves(offset);
        offset += lineSize;
    }
}


void StavesBackgroundPainter::paintBackgroundStaves(double offset) {
    Util::cairo_set_source_rgbi(cr, this->foregroundColor1);
    cairo_set_line_width(cr, lineWidth * lineWidthFactor);

    double staveOffset = offset;
    for (int j = 0; j < 5; j++) {
        cairo_move_to(cr, this->borderSize, staveOffset);
        cairo_line_to(cr, this->width - this->borderSize, staveOffset);
        staveOffset += this->staveDistance;
    }

    cairo_move_to(cr, this->borderSize, offset - (lineWidth * lineWidthFactor) / 2);
    cairo_line_to(cr, this->borderSize, offset + 4 * staveDistance + (lineWidth * lineWidthFactor) / 2);

    cairo_move_to(cr, this->width - this->borderSize, offset - (lineWidth * lineWidthFactor) / 2);
    cairo_line_to(cr, this->width - this->borderSize, offset + 4 * staveDistance + (lineWidth * lineWidthFactor) / 2);

    cairo_stroke(cr);
}
