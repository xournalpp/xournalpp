#include "BaseBackgroundPainter.h"

#include "util/Util.h"

BaseBackgroundPainter::BaseBackgroundPainter() { resetConfig(); }

BaseBackgroundPainter::~BaseBackgroundPainter() = default;

/**
 * Set a factor to draw the lines bolder, for previews
 */
void BaseBackgroundPainter::setLineWidthFactor(double factor) { this->lineWidthFactor = factor; }

void BaseBackgroundPainter::resetConfig() {
    // Overwritten from the subclasses
}

auto BaseBackgroundPainter::alternativeColor(Color color1, Color color2) const -> Color {
    auto backgroundColor = this->page->getBackgroundColor();

    auto greyscale = [](Color const& color) { return uint32_t(color.red + color.green + color.blue) / 3; };

    return greyscale(backgroundColor) < 0x80U ? color2 : color1;
}

auto BaseBackgroundPainter::getForegroundColor1() const -> Color {
    uint32_t temp{};

    const Color foregroundColor = this->config->loadValueHex("f1", temp) ? Color(temp) : this->defaultForegroundColor1;
    const Color altForegroundColor =
            this->config->loadValueHex("af1", temp) ? Color(temp) : this->defaultAlternativeForegroundColor1;

    return this->alternativeColor(foregroundColor, altForegroundColor);
}

auto BaseBackgroundPainter::getForegroundColor2() const -> Color {
    uint32_t temp{};

    const Color foregroundColor = this->config->loadValueHex("f2", temp) ? Color(temp) : this->defaultForegroundColor2;
    const Color altForegroundColor =
            this->config->loadValueHex("af2", temp) ? Color(temp) : this->defaultAlternativeForegroundColor2;

    return this->alternativeColor(foregroundColor, altForegroundColor);
}

void BaseBackgroundPainter::paint(cairo_t* cr, PageRef page, BackgroundConfig* config) {
    this->cr = cr;
    this->page = page;
    this->config = config;

    this->width = page->getWidth();
    this->height = page->getHeight();

    this->foregroundColor1 = this->getForegroundColor1();
    this->foregroundColor2 = this->getForegroundColor2();

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
