#include "GraphBackgroundPainter.h"

#include <cmath>

#include "Util.h"

GraphBackgroundPainter::GraphBackgroundPainter() = default;

GraphBackgroundPainter::~GraphBackgroundPainter() = default;

/**
 * Set the Graph line color to a lower contrast alternative if a black background is used
 */
void GraphBackgroundPainter::updateGraphColor() {
    Color backgroundColor = this->page->getBackgroundColor();

    auto greyscale = [](Color color) {
        return ((0xffU & uint32_t(color)) + (0xffU & (uint32_t(color) >> 8U)) + (0xffU & (uint32_t(color) >> 16U))) / 3;
    };

    this->foregroundColor1 = greyscale(backgroundColor) < 0x80U ? 0x202020U : 0xBDBDBDU;
}

void GraphBackgroundPainter::resetConfig() {
    this->foregroundColor1 = 0xBDBDBDU;
    this->lineWidth = 0.5;
    this->drawRaster1 = 14.17;
    this->margin1 = 0;
    this->roundMargin = 0;
}

auto GraphBackgroundPainter::getUnitSize() -> double { return this->drawRaster1; }

void GraphBackgroundPainter::paint() {
    this->updateGraphColor();
    paintBackgroundColor();
    paintBackgroundGraph();
}

void GraphBackgroundPainter::paintBackgroundGraph() {
    Util::cairo_set_source_rgbi(cr, this->foregroundColor1);

    cairo_set_line_width(cr, lineWidth * lineWidthFactor);
    double marginTopBottom = margin1;
    double marginLeftRight = margin1;
    double snappingOffset = 2.5;

    if (roundMargin) {
        double w = width - 2 * marginLeftRight;
        double r = w - floor(w / drawRaster1) * drawRaster1;
        marginLeftRight += r / 2;
        // startX = marginLeftRight;

        double h = height - 2 * marginTopBottom;
        r = h - floor(h / drawRaster1) * drawRaster1;
        marginTopBottom += r / 2;
        // startY = marginTopBottom;
    }

    auto pos = [dr1 = drawRaster1](int i) { return dr1 + i * dr1; };

    for (int x = 0; pos(x) < width; ++x) {
        if (pos(x) < margin1 || pos(x) > (width - margin1)) {
            continue;
        }
        cairo_move_to(cr, pos(x), marginTopBottom - snappingOffset);
        cairo_line_to(cr, pos(x), height - marginTopBottom - snappingOffset);
    }

    for (int y = 0; pos(y) < height; ++y) {
        if (pos(y) < margin1 || pos(y) > (height - marginTopBottom)) {
            continue;
        }

        cairo_move_to(cr, marginLeftRight, pos(y));
        cairo_line_to(cr, width - marginLeftRight, pos(y));
    }

    cairo_stroke(cr);
}
