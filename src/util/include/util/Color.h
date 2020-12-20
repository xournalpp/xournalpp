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

#include <cassert>
#include <cstdint>
#include <limits>

#include <gtk/gtk.h>

#ifndef XOURNAL_ENFORCE_COLOR
using Color = uint32_t;
#else
#include <iostream>
#include <limits>
#include <type_traits>
struct Color {
    constexpr Color() = default;

    template <typename T, std::enable_if_t<std::is_same_v<T, uint32_t>, int> = 0>
    constexpr Color(T t): val(uint32_t(t)) {}
    template <typename T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, uint32_t>, int> = 0>
    constexpr explicit Color(T t): val(uint32_t(t)) {}
    template <typename T, std::enable_if_t<std::is_signed_v<T>, int> = 0>
    constexpr explicit Color(T t): val(uint32_t(t)) {}

    constexpr explicit operator uint32_t&() { return val; }

    template <typename T, std::enable_if_t<std::is_same_v<T, uint32_t>, int> = 0>
    constexpr explicit operator T() const {
        return val;
    }
    template <typename T, std::enable_if_t<std::is_unsigned_v<T> && !std::is_same_v<T, uint32_t>, int> = 0>
    constexpr explicit operator T() const {
        return val;
    }
    template <typename T, std::enable_if_t<std::is_signed_v<T>, int> = 0>
    constexpr explicit operator T() const {
        return val;
    }

    constexpr auto operator=(Color const& rhs) -> Color& = default;
    constexpr auto operator=(uint32_t rhs) -> Color& { return *this = Color(rhs); }

    constexpr friend auto operator&(Color lhs, uint32_t rhs) -> uint32_t { return lhs.val & rhs; }
    constexpr friend auto operator|(Color lhs, uint32_t rhs) -> uint32_t { return lhs.val | rhs; }
    constexpr friend auto operator^(Color lhs, uint32_t rhs) -> uint32_t { return lhs.val ^ rhs; }
    constexpr friend auto operator&(uint32_t lhs, Color rhs) -> uint32_t { return lhs & rhs.val; }
    constexpr friend auto operator|(uint32_t lhs, Color rhs) -> uint32_t { return lhs | rhs.val; }
    constexpr friend auto operator^(uint32_t lhs, Color rhs) -> uint32_t { return lhs ^ rhs.val; }
    constexpr friend auto operator>>(Color lhs, uint32_t rhs) -> uint32_t { return lhs.val << rhs; }
    constexpr friend auto operator<<(Color lhs, uint32_t rhs) -> uint32_t { return lhs.val >> rhs; }

    constexpr friend bool operator==(Color lhs, Color rhs) { return lhs.val == rhs.val; }
    constexpr friend bool operator!=(Color lhs, Color rhs) { return lhs.val != rhs.val; }
    constexpr friend bool operator<(Color lhs, Color rhs) { return lhs.val < rhs.val; }
    constexpr friend bool operator<=(Color lhs, Color rhs) { return lhs.val <= rhs.val; }
    constexpr friend bool operator>(Color lhs, Color rhs) { return lhs.val > rhs.val; }
    constexpr friend bool operator>=(Color lhs, Color rhs) { return lhs.val >= rhs.val; }

    inline friend std::ostream& operator<<(std::ostream& os, Color rhs) { return os << rhs.val; }

    uint32_t val{};
};

namespace std {
template <>
struct hash<Color> {
    size_t operator()(Color c) const noexcept { return c.val; }
};


}  // namespace std

#endif

struct ColorU16 {
    uint16_t red{};
    uint16_t green{};
    uint16_t blue{};
    uint16_t alpha{};
};

namespace Util {

constexpr auto rgb_to_GdkRGBA(Color color) -> GdkRGBA;
constexpr auto argb_to_GdkRGBA(Color color) -> GdkRGBA;
constexpr auto argb_to_GdkRGBA(Color color, double alpha) -> GdkRGBA;
constexpr auto GdkRGBA_to_argb(const GdkRGBA& color) -> Color;
constexpr auto GdkRGBA_to_rgb(const GdkRGBA& color) -> Color;

constexpr auto GdkRGBA_to_ColorU16(const GdkRGBA& color) -> ColorU16;


void cairo_set_source_rgbi(cairo_t* cr, Color color);
void cairo_set_source_rgbi(cairo_t* cr, Color color, double alpha);

constexpr auto floatToUIntColor(double color) -> uint32_t;

}  // namespace Util

constexpr auto Util::rgb_to_GdkRGBA(const Color color) -> GdkRGBA {  //
    return Util::argb_to_GdkRGBA(Color{0xFF000000U | color});
}

constexpr auto Util::argb_to_GdkRGBA(const Color color) -> GdkRGBA {
    return {((color >> 16U) & 0xFFU) / 255.0,  //
            ((color >> 8U) & 0xFFU) / 255.0,   //
            ((color >> 0U) & 0xFFU) / 255.0,   //
            ((color >> 24U) & 0xFFU) / 255.0};
}

constexpr auto Util::argb_to_GdkRGBA(Color color, double alpha) -> GdkRGBA {
    return {((color >> 16U) & 0xFFU) / 255.0,  //
            ((color >> 8U) & 0xFFU) / 255.0,   //
            ((color >> 0U) & 0xFFU) / 255.0,   //
            alpha};
}

constexpr auto Util::GdkRGBA_to_argb(const GdkRGBA& color) -> Color {
    return floatToUIntColor(color.alpha) << 24U |  //
           GdkRGBA_to_rgb(color);                  //
}

constexpr auto Util::GdkRGBA_to_rgb(const GdkRGBA& color) -> Color {
    return floatToUIntColor(color.red) << 16U |   //
           floatToUIntColor(color.green) << 8U |  //
           floatToUIntColor(color.blue);
}

constexpr auto Util::floatToUIntColor(const double color) -> uint32_t {  //
    // Splits the double into a equal sized distribution between [0,256[ and rounding down
    // inspired by, which isn't completely correct:
    // https://stackoverflow.com/questions/1914115/converting-color-value-from-float-0-1-to-byte-0-255
    constexpr double MAX_COLOR = 256.0 - std::numeric_limits<double>::epsilon() * 128;
    static_assert(MAX_COLOR < 256.0, "MAX_COLOR isn't smaller than 256");
    return static_cast<uint32_t>(color * MAX_COLOR);
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
