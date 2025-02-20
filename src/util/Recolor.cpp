#include "util/Recolor.h"

#include "util/Color.h"

Recolor::Recolor(const ColorU8& light, const ColorU8& dark): dark(dark), light(light) { recalcDiffAndOff(); }

const ColorU8& Recolor::getDark() const { return dark; }

const ColorU8& Recolor::getLight() const { return light; }

void Recolor::recalcDiffAndOff() {
    difference = ColorU8{
            static_cast<uint8_t>(std::abs(static_cast<int16_t>(dark.red) - static_cast<int16_t>(light.red))),
            static_cast<uint8_t>(std::abs(static_cast<int16_t>(dark.green) - static_cast<int16_t>(light.green))),
            static_cast<uint8_t>(std::abs(static_cast<int16_t>(dark.blue) - static_cast<int16_t>(light.blue)))};

    offset = ColorU8{std::min(dark.red, light.red), std::min(dark.green, light.green), std::min(dark.blue, light.blue)};
}

ColorU8 Recolor::convertColor(const ColorU8& other) const {
    return ColorU8{
            static_cast<uint8_t>(std::min(
                    255, std::min(255, (255 - static_cast<int>(other.red)) * static_cast<int>(difference.red) / 255) +
                                 offset.red)),
            static_cast<uint8_t>(std::min(255, std::min(255, (255 - static_cast<int>(other.green)) *
                                                                     static_cast<int>(difference.green) / 255) +
                                                       offset.green)),
            static_cast<uint8_t>(std::min(
                    255, std::min(255, (255 - static_cast<int>(other.blue)) * static_cast<int>(difference.blue) / 255) +
                                 offset.blue)),
    };
}

void Recolor::recolorCurrentCairoRegion(cairo_t* cr) const {
    // Apply inversion
    cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    cairo_set_source_rgb(cr, 1., 1., 1.);
    cairo_paint(cr);

    // Scale the spectrum
    cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
    Util::cairo_set_source_rgbi(cr, difference);
    cairo_paint(cr);

    // Move the spectrum
    cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
    Util::cairo_set_source_rgbi(cr, offset);
    cairo_paint(cr);
}
