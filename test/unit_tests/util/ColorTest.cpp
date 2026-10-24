/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "util/Color.h"

using namespace std;

bool operator==(GdkRGBA const& lhs, GdkRGBA const& rhs) noexcept {
    auto tieer = [](GdkRGBA const& val) { return std::tie(val.red, val.blue, val.green, val.alpha); };
    return tieer(lhs) == tieer(rhs);
}

std::ostream& operator<<(std::ostream& os, GdkRGBA const& val) {
    return os << "GdkRGBA{" << val.red << "," << val.green << "," << val.blue << "," << val.alpha << "}";
}


TEST(UtilColor, testColorToRGB) {
    Color color1{0U};
    Color color2{0xffffffU};
    Color color3{0x7f7f7fU};
    Color color4{0x808080U};
    auto rgb1 = GdkRGBA{0.0, 0.0, 0.0, 1.0};
    auto rgb2 = GdkRGBA{1, 1, 1, 1.0};
    auto rgb3 = GdkRGBA{0.5, 0.5, 0.5, 1.0};
    auto rgb_t1 = Util::rgb_to_GdkRGBA(color1);
    EXPECT_EQ(rgb1, rgb_t1);
    auto rgb_t2 = Util::rgb_to_GdkRGBA(color2);
    EXPECT_EQ(rgb2, rgb_t2);
    auto rgb_t4 = Util::rgb_to_GdkRGBA(color4);
    auto rgb_t3 = Util::rgb_to_GdkRGBA(color3);
    auto rgb_t5 = GdkRGBA{(rgb_t3.red + rgb_t4.red) / 2, (rgb_t3.green + rgb_t4.green) / 2,
                          (rgb_t3.blue + rgb_t4.blue) / 2, 1};
    EXPECT_EQ(rgb3, rgb_t5);
}

TEST(UtilColor, testColorToRGBAndBack) {
    for (size_t i = 0U; i < 256U; ++i) {
        uint8_t ii = static_cast<uint8_t>(i);
        Color color{ii, ii, ii};
        Color color2{ii, ii, ii, ii};
        EXPECT_EQ(color, Util::GdkRGBA_to_rgb(Util::rgb_to_GdkRGBA(color)));
        EXPECT_EQ(color2, Util::GdkRGBA_to_argb(Util::argb_to_GdkRGBA(color2)));
    }
}

TEST(UtilColor, RelativeLuminanceAndHighlighterOperator) {
    EXPECT_NEAR(Colors::black.getRelativeLuminance(), 0.0, 1e-4);
    EXPECT_NEAR(Colors::white.getRelativeLuminance(), 1.0, 1e-4);

    // Individual BT.709 channel weight verification
    constexpr Color pureRed{0xffff0000U};
    constexpr Color pureGreen{0xff00ff00U};
    constexpr Color pureBlue{0xff0000ffU};
    EXPECT_NEAR(pureRed.getRelativeLuminance(), 0.2126, 1e-4);
    EXPECT_NEAR(pureGreen.getRelativeLuminance(), 0.7152, 1e-4);
    EXPECT_NEAR(pureBlue.getRelativeLuminance(), 0.0722, 1e-4);

    EXPECT_TRUE(Colors::black.isDark());
    EXPECT_FALSE(Colors::white.isDark());

    constexpr Color justAboveHalf{0xff808080U}; // RGB(128,128,128) -> Luminance ≈ 0.5019
    constexpr Color justBelowHalf{0xff787878U}; // RGB(120,120,120) -> Luminance ≈ 0.4705
    EXPECT_FALSE(justAboveHalf.isDark());
    EXPECT_TRUE(justBelowHalf.isDark());

    EXPECT_EQ(Colors::white.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
    EXPECT_EQ(Colors::black.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);
    EXPECT_EQ(justAboveHalf.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
    EXPECT_EQ(justBelowHalf.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);
}