#include "util/Recolor.h"

#include "util/Color.h"

Recolor::Recolor(const ColorU8& light, const ColorU8& dark, bool keepHues): dark(dark), light(light), keepHues(keepHues) { recalcDiffAndOff(); }

const ColorU8& Recolor::getDark() const { return dark; }

const ColorU8& Recolor::getLight() const { return light; }

bool Recolor::getKeepHues() const { return keepHues; }

void Recolor::recalcDiffAndOff() {
    difference = ColorU8{
        static_cast<uint8_t>(std::abs(static_cast<int16_t>(dark.red) - static_cast<int16_t>(light.red))),
            static_cast<uint8_t>(std::abs(static_cast<int16_t>(dark.green) - static_cast<int16_t>(light.green))),
            static_cast<uint8_t>(std::abs(static_cast<int16_t>(dark.blue) - static_cast<int16_t>(light.blue)))};

    offset = ColorU8{std::min(dark.red, light.red), std::min(dark.green, light.green), std::min(dark.blue, light.blue)};
    ref = ColorU8{
        static_cast<uint8_t>((light.red < dark.red ? 255 : 0)),
            static_cast<uint8_t>((light.green < dark.green ? 255 : 0)),
            static_cast<uint8_t>((light.blue < dark.blue ? 255 : 0)),
    };
}

ColorU8 Recolor::convertColor(const ColorU8& other) const {
    ColorU8 inverted = other;
    if (keepHues) {
        int cmin = std::min(std::min(other.red,other.green),other.blue);
        int cmax = std::max(std::max(other.red,other.green),other.blue);
        inverted = { static_cast<uint8_t>(cmin + cmax - other.red), static_cast<uint8_t>(cmin + cmax - other.green), static_cast<uint8_t>(cmin + cmax - other.blue) };
    }
    return ColorU8{
        static_cast<uint8_t>(
                std::min(255, std::min(255, std::abs(static_cast<int>(ref.red) - static_cast<int>(inverted.red)) *
                        static_cast<int>(difference.red) / 255) +
                    offset.red)),
            static_cast<uint8_t>(
                    std::min(255, std::min(255, std::abs(static_cast<int>(ref.green) - static_cast<int>(inverted.green)) *
                            static_cast<int>(difference.green) / 255) +
                        offset.green)),
            static_cast<uint8_t>(
                    std::min(255, std::min(255, std::abs(static_cast<int>(ref.blue) - static_cast<int>(inverted.blue)) *
                            static_cast<int>(difference.blue) / 255) +
                        offset.blue)),
    };
}

void Recolor::recolorCurrentCairoRegion(cairo_t* cr) const {

    if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) return;

    cairo_identity_matrix(cr);

    if (keepHues) {
        // Invert hues by copying target surface, inverting, and applying hue operator...
        cairo_surface_t *target = cairo_get_group_target(cr);
        cairo_surface_t *copy = cairo_surface_create_similar(target, cairo_surface_get_content(target), cairo_image_surface_get_width(target), cairo_image_surface_get_height(target));

        cairo_t *cr_new = cairo_create(copy);
        cairo_set_source_surface(cr_new,target,0,0);
        cairo_paint(cr_new);

        cairo_set_operator(cr_new, CAIRO_OPERATOR_DIFFERENCE);
        Util::cairo_set_source_rgbi(cr_new, ColorU8{255,255,255});
        cairo_paint(cr_new);

        // Copy inverted hues back on
        cairo_set_operator(cr, CAIRO_OPERATOR_HSL_HUE);
        cairo_set_source_surface(cr,copy,0,0);
        cairo_paint(cr);

        cairo_surface_destroy(copy);
        cairo_destroy(cr_new);
    }

    // Apply (full) inversion
    cairo_set_operator(cr, CAIRO_OPERATOR_DIFFERENCE);
    Util::cairo_set_source_rgbi(cr, ref);
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
