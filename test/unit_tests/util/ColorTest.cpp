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

TEST(ColorU8LuminanceTest, BlackAndWhiteExtremes) {
    constexpr ColorU8 black{0xff000000U};
    constexpr ColorU8 white{0xffffffffU};

    EXPECT_NEAR(black.getRelativeLuminance(), 0.0, 1e-6);
    EXPECT_FALSE(black.isLight());
    EXPECT_EQ(black.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);

    EXPECT_NEAR(white.getRelativeLuminance(), 1.0, 1e-6);
    EXPECT_TRUE(white.isLight());
    EXPECT_EQ(white.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
}

TEST(ColorU8LuminanceTest, ChannelWeightsAndThreshold) {
    // BT.709 weights: R = 0.2126, G = 0.7152, B = 0.0722
    constexpr ColorU8 red{0xffff0000U};
    constexpr ColorU8 green{0xff00ff00U};
    constexpr ColorU8 blue{0xff0000ffU};

    EXPECT_NEAR(red.getRelativeLuminance(), 0.2126, 1e-4);
    EXPECT_FALSE(red.isLight());
    EXPECT_EQ(red.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);

    EXPECT_NEAR(green.getRelativeLuminance(), 0.7152, 1e-4);
    EXPECT_TRUE(green.isLight());
    EXPECT_EQ(green.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    EXPECT_NEAR(blue.getRelativeLuminance(), 0.0722, 1e-4);
    EXPECT_FALSE(blue.isLight());
    EXPECT_EQ(blue.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);
}

TEST(ColorU8LuminanceTest, IsLightThresholdBoundary) {
    // isLight() checks getRelativeLuminance() > 0.5
    constexpr ColorU8 gray80{0xff808080U};  // 128/255 -> ~0.50196 > 0.5
    EXPECT_GT(gray80.getRelativeLuminance(), 0.5);
    EXPECT_TRUE(gray80.isLight());
    EXPECT_EQ(gray80.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    constexpr ColorU8 gray7F{0xff7f7f7fU};  // 127/255 -> ~0.49804 <= 0.5
    EXPECT_LE(gray7F.getRelativeLuminance(), 0.5);
    EXPECT_FALSE(gray7F.isLight());
    EXPECT_EQ(gray7F.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);
}

TEST(ColorU8LuminanceTest, IgnoresAlphaChannel) {
    constexpr ColorU8 transparentDark{0x00101010U};
    constexpr ColorU8 opaqueDark{0xff101010U};

    EXPECT_DOUBLE_EQ(transparentDark.getRelativeLuminance(), opaqueDark.getRelativeLuminance());
    EXPECT_EQ(transparentDark.isLight(), opaqueDark.isLight());
    EXPECT_EQ(transparentDark.getHighlighterOperator(), opaqueDark.getHighlighterOperator());
}
