#include "BaseBackgroundPainter.h"

#include "Util.h"

BaseBackgroundPainter::BaseBackgroundPainter() { resetConfig(); }

BaseBackgroundPainter::~BaseBackgroundPainter() = default;

int BaseBackgroundPainter::alternativeColor(int color1, int color2) {
    auto backgroundColor = this->page->getBackgroundColor();

    auto greyscale = [](uint32_t color) {
        return ((0xff & color) + (0xff & (color >> 8)) + (0xff & (color >> 16))) / 3;
    };

    return greyscale(backgroundColor) < 0x80 ? color2 : color1;
}

/**
 * Set a factor to draw the lines bolder, for previews
 */
void BaseBackgroundPainter::setLineWidthFactor(double factor) { this->lineWidthFactor = factor; }

void BaseBackgroundPainter::resetConfig() {
    // Overwritten from the subclasses
}

void BaseBackgroundPainter::paint(cairo_t* cr, PageRef page, BackgroundConfig* config) {
    this->cr = cr;
    this->page = page;
    this->config = config;

    this->width = page->getWidth();
    this->height = page->getHeight();

    this->config->loadValueHex("f1", this->foregroundColor1);
    this->config->loadValueHex("f2", this->foregroundColor2);

    this->config->loadValueHex("af1", this->alternativeForegroundColor1);
    this->config->loadValueHex("af2", this->alternativeForegroundColor2);

    this->config->loadValue("lw", this->lineWidth);
    this->config->loadValue("r1", this->drawRaster1);

    this->config->loadValue("m1", this->margin1);
    this->config->loadValue("rm", this->roundMargin);

    // If the raster is two small, we get an andless loop....
    if (drawRaster1 < 1) {
        drawRaster1 = 1;
    }

    paint();

    this->cr = nullptr;
    this->config = nullptr;
}

void BaseBackgroundPainter::paint() { paintBackgroundColor(); }

void BaseBackgroundPainter::paintBackgroundColor() {
    Util::cairo_set_source_rgbi(cr, page->getBackgroundColor());

    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
}
