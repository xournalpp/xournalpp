#include <cairo.h>
#include <gtest/gtest.h>

#include "model/PageType.h"
#include "model/XojPage.h"
#include "util/Color.h"

namespace {

TEST(XojPageHighlighterOperatorTest, DefaultPageUsesLightBackgroundOperator) {
    // XojPage defaults bgType to Lined and backgroundColor to Colors::white
    const XojPage page(500.0, 700.0);
    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
}

TEST(XojPageHighlighterOperatorTest, StandardBackgroundColorSelectsOperatorByLuminance) {
    XojPage page(500.0, 700.0);
    page.setBackgroundType(PageType(PageTypeFormat::Plain));

    page.setBackgroundColor(Colors::black);
    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);

    page.setBackgroundColor(Colors::white);
    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
}

TEST(XojPageHighlighterOperatorTest, PdfBackgroundForcesMultiplyEvenWithDarkColor) {
    XojPage page(500.0, 700.0);
    page.setBackgroundColor(Colors::black);
    page.setBackgroundPdfPageNr(0);

    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
}

TEST(XojPageHighlighterOperatorTest, ImageBackgroundForcesMultiplyEvenWithDarkColor) {
    XojPage page(500.0, 700.0);
    page.setBackgroundColor(Colors::black);
    page.setBackgroundType(PageType(PageTypeFormat::Image));

    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
}

TEST(XojPageHighlighterOperatorTest, SwitchingBackgroundTypesUpdatesOperator) {
    XojPage page(500.0, 700.0);
    page.setBackgroundColor(Colors::black);
    page.setBackgroundType(PageType(PageTypeFormat::Plain));
    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);

    page.setBackgroundType(PageType(PageTypeFormat::Image));
    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    page.setBackgroundType(PageType(PageTypeFormat::Graph));
    EXPECT_EQ(page.getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);
}

}  // namespace
