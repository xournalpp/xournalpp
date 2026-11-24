#include <cairo.h>
#include <gtest/gtest.h>

#include "model/PageType.h"
#include "model/XojPage.h"
#include "util/Color.h"

namespace {

TEST(XojPageHighlighterOperatorTest, DefaultPageUsesLightBackgroundOperator) {
    // XojPage defaults bgType to Lined and backgroundColor to Colors::white
    const XojPage page(500.0, 700.0);  //[cite: 5, 6]
    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);  //[cite: 5, 6]
}

TEST(XojPageHighlighterOperatorTest, StandardBackgroundColorSelectsOperatorByLuminance) {
    XojPage page(500.0, 700.0);  //[cite: 5, 6]
    page.setBackgroundType(PageType(PageTypeFormat::Plain));  //[cite: 5, 6]

    page.setBackgroundColor(Colors::black);  //[cite: 4, 6]
    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_SCREEN);  //[cite: 4, 6]

    page.setBackgroundColor(Colors::white);  //[cite: 4, 6]
    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);  //[cite: 4, 6]
}

TEST(XojPageHighlighterOperatorTest, PdfBackgroundForcesMultiplyEvenWithDarkColor) {
    XojPage page(500.0, 700.0);  //[cite: 5, 6]
    page.setBackgroundColor(Colors::black);  //[cite: 4, 6]
    page.setBackgroundPdfPageNr(0);  // sets format to PageTypeFormat::Pdf[cite: 6]

    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);  //[cite: 5, 6]
}

TEST(XojPageHighlighterOperatorTest, ImageBackgroundForcesMultiplyEvenWithDarkColor) {
    XojPage page(500.0, 700.0);  //[cite: 5, 6]
    page.setBackgroundColor(Colors::black);  //[cite: 4, 6]
    page.setBackgroundType(PageType(PageTypeFormat::Image));  //[cite: 5, 6]

    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);  //[cite: 5, 6]
}

TEST(XojPageHighlighterOperatorTest, SwitchingBackgroundTypesUpdatesOperator) {
    XojPage page(500.0, 700.0);  //[cite: 5, 6]
    page.setBackgroundColor(Colors::black);  //[cite: 4, 6]
    page.setBackgroundType(PageType(PageTypeFormat::Plain));  //[cite: 5, 6]
    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_SCREEN);  //[cite: 4, 6]

    page.setBackgroundType(PageType(PageTypeFormat::Image));  //[cite: 5, 6]
    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);  //[cite: 5, 6]

    page.setBackgroundType(PageType(PageTypeFormat::Graph));  //[cite: 5, 6]
    EXPECT_EQ(page.getPageHighlighterOperator(), CAIRO_OPERATOR_SCREEN);  //[cite: 4, 6]
}

} //namespace