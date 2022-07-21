/*
 * Xournal++
 *
 * Color utility, does color conversions
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cstdint>      // for uint32_t, uint8_t, uint16_t
#include <cstring>      // for size_t
#include <iostream>     // for istream, ostream, basic_istream<>::__istream_...
#include <limits>       // for numeric_limits
#include <string>       // for string
#include <string_view>  // for hash

#include <cairo.h>    // for cairo_t
#include <gdk/gdk.h>  // for GdkRGBA


struct ColorU8 {
    uint8_t red{};
    uint8_t green{};
    uint8_t blue{};
    uint8_t alpha{};

    constexpr ColorU8() = default;
    constexpr ColorU8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0): red(r), green(g), blue(b), alpha(a) {}
    constexpr explicit ColorU8(uint32_t val):
            // Legacy ordering for serdes: 0xAARRGGBB
            red(uint8_t((val >> 16U) & 0xffU)),
            green(uint8_t((val >> 8U) & 0xffU)),
            blue(uint8_t((val >> 0U) & 0xffU)),
            alpha(uint8_t((val >> 24U) & 0xffU)) {}

    constexpr explicit operator uint32_t() const {
        // Legacy ordering for serdes: 0xAARRGGBB
        uint32_t val = (uint32_t(red) << 16U) | (uint32_t(green) << 8U) | uint32_t(blue) | (uint32_t(alpha) << 24U);
        return val;
    }

    constexpr auto operator=(uint32_t rhs) -> ColorU8& {
        *this = ColorU8(rhs);
        return *this;
    }

    constexpr friend bool operator==(ColorU8 const& lhs, ColorU8 const& rhs) { return uint32_t(lhs) == uint32_t(rhs); }
    constexpr friend bool operator!=(ColorU8 const& lhs, ColorU8 const& rhs) { return uint32_t(lhs) != uint32_t(rhs); }
    constexpr friend bool operator<(ColorU8 const& lhs, ColorU8 const& rhs) { return uint32_t(lhs) < uint32_t(rhs); }

    constexpr friend bool operator==(ColorU8 const& lhs, uint32_t rhs) { return uint32_t(lhs) == rhs; }
    constexpr friend bool operator!=(ColorU8 const& lhs, uint32_t rhs) { return uint32_t(lhs) != rhs; }
    constexpr friend bool operator<(ColorU8 const& lhs, uint32_t rhs) { return uint32_t(lhs) < rhs; }

    inline friend std::ostream& operator<<(std::ostream& os, ColorU8 rhs) { return os << uint32_t(rhs); }
    inline friend std::istream& operator>>(std::istream& is, ColorU8& rhs) {
        uint32_t val{};
        is >> val;
        rhs = val;
        return is;
    };

    constexpr auto isLight() const -> bool { return uint32_t(red) + uint32_t(green) + uint32_t(blue) > 0x180U; }
};

static_assert(sizeof(ColorU8) == sizeof(uint32_t), "Color is not 32 bit");
using Color = ColorU8;

namespace std {
template <>
struct hash<Color> {
    size_t operator()(Color c) const noexcept { return uint32_t(c); }
};

}  // namespace std

struct ColorU16 {
    uint16_t red{};
    uint16_t green{};
    uint16_t blue{};
    uint16_t alpha{};
};

inline std::ostream& operator<<(std::ostream& os, ColorU16 rhs) {
    return os << rhs.red << rhs.green << rhs.blue << rhs.alpha;
}
inline std::istream& operator>>(std::istream& is, ColorU16& rhs) {
    return is >> rhs.red >> rhs.green >> rhs.blue >> rhs.alpha;
}

namespace Util {

constexpr auto rgb_to_GdkRGBA(Color color) -> GdkRGBA;
constexpr auto argb_to_GdkRGBA(Color color) -> GdkRGBA;
constexpr auto argb_to_GdkRGBA(Color color, double alpha) -> GdkRGBA;
constexpr auto GdkRGBA_to_argb(const GdkRGBA& color) -> Color;
constexpr auto GdkRGBA_to_rgb(const GdkRGBA& color) -> Color;

constexpr auto ColorU16_to_argb(const ColorU16& color) -> Color;
constexpr auto argb_to_ColorU16(const Color& color) -> ColorU16;
constexpr auto GdkRGBA_to_ColorU16(const GdkRGBA& color) -> ColorU16;

/// Set the color of a cairo context -- uses alpha as alpha value
void cairo_set_source_rgbi(cairo_t* cr, Color color, double alpha = 1.0);

/// Set the color of a cairo context -- uses color.alpha as alpha value
void cairo_set_source_argb(cairo_t* cr, Color color);  // Use color.alpha

constexpr auto floatToUIntColor(double color) -> uint8_t;

/**
 * @param color Color to convert to grayscale.
 * @return Given color converted to grayscale 1.0 -> white, 0.0 -> black.
 */
float as_grayscale_color(Color color);

/**
 *  Get the fraction by which the grayscale value of color1 contrasts
 * with color2 (0.0 = no contrast, 1.0 = maximum contrast).
 * Does not take a color's alpha channel into account.
 * @return Scale factor by which the two given colors differ. Must be in
 *          [0.0, 1.0].
 */
float get_color_contrast(Color color1, Color color2);

/**
 * @param rgb Color to get a representation for.
 * @return a CSS-style representation of the color, in hex. For example,
 *          red might be #ff0000, green, #00ff00, and blue, #0000ff.
 */
std::string rgb_to_hex_string(Color rgb);

}  // namespace Util

constexpr auto Util::rgb_to_GdkRGBA(Color color) -> GdkRGBA {  //
    color.alpha = 0xFF;
    return Util::argb_to_GdkRGBA(color);
}

constexpr auto Util::argb_to_GdkRGBA(const Color color) -> GdkRGBA {
    return {color.red / 255.0,    //
            color.green / 255.0,  //
            color.blue / 255.0,   //
            color.alpha / 255.0};
}

constexpr auto Util::argb_to_GdkRGBA(Color color, double alpha) -> GdkRGBA {
    return {color.red / 255.0,    //
            color.green / 255.0,  //
            color.blue / 255.0,   //
            alpha};
}

constexpr auto Util::GdkRGBA_to_argb(const GdkRGBA& color) -> Color {
    auto ret = GdkRGBA_to_rgb(color);
    ret.alpha = floatToUIntColor(color.alpha);
    return ret;
}

constexpr auto Util::GdkRGBA_to_rgb(const GdkRGBA& color) -> Color {
    return Color{floatToUIntColor(color.red),    //
                 floatToUIntColor(color.green),  //
                 floatToUIntColor(color.blue)};
}

constexpr auto Util::argb_to_ColorU16(const Color& color) -> ColorU16 {
    /* 0xff should map to 0xffff in 16 bit. Therefore multipliing by 257 instead of 256*/
    return {static_cast<uint16_t>((color.red << 8U) + color.red),
            static_cast<uint16_t>((color.green << 8U) + color.green),
            static_cast<uint16_t>((color.blue << 8U) + color.blue),
            static_cast<uint16_t>((color.alpha << 8U) + color.alpha)};
}

constexpr auto Util::ColorU16_to_argb(const ColorU16& color) -> Color {
    return Color{static_cast<uint8_t>(color.red >> 8U), static_cast<uint8_t>(color.green >> 8U),
                 static_cast<uint8_t>(color.blue >> 8U), static_cast<uint8_t>(color.alpha >> 8U)};
}

constexpr auto Util::floatToUIntColor(const double color) -> uint8_t {
    /*
     * Splits the double into a equal sized distribution between [0,256[ and rounding down
     * inspired by, which isn't completely correct:
     * https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255
     */
    constexpr double MAX_COLOR = 256.0 - std::numeric_limits<double>::epsilon() * 128;
    static_assert(MAX_COLOR < 256.0, "MAX_COLOR isn't smaller than 256");
    return static_cast<uint8_t>(color * MAX_COLOR);
}

constexpr auto Util::GdkRGBA_to_ColorU16(const GdkRGBA& color) -> ColorU16 {
    auto floatToColorU16 = [](double color) {
        constexpr double MAX_COLOR = 65536.0 - std::numeric_limits<double>::epsilon() * (65536.0 / 2.0);
        static_assert(MAX_COLOR < 65536.0, "MAX_COLOR isn't smaller than 65536");
        return static_cast<uint16_t>(color * MAX_COLOR);
    };

    return {floatToColorU16(color.red),    //
            floatToColorU16(color.green),  //
            floatToColorU16(color.blue),   //
            floatToColorU16(color.alpha)};
}
