#include <gtest/gtest.h>

#include "model/PageType.h"
#include "model/XojPage.h"
#include "util/Color.h"

TEST(XojPageTest, SetBackgroundTypeResetsColorForSpecialBackgrounds) {
    XojPage page(595.0, 842.0);  // Standard page dimensions

    // 1. Set a dark background color initially
    page.setBackgroundColor(Color(0x1E1E1E));
    EXPECT_EQ(page.getBackgroundColor(), Color(0x1E1E1E));

    // 2. Transition background type to Image
    PageType imageBg(PageTypeFormat::Image);
    page.setBackgroundType(imageBg);

    // 3. Verify color is reset to default white
    EXPECT_EQ(page.getBackgroundColor(), Colors::white);

    // 4. Test PDF background transition as well
    page.setBackgroundColor(Color(0x000000));
    PageType pdfBg(PageTypeFormat::Pdf);
    page.setBackgroundType(pdfBg);

    // 5. Verify color is reset to white again
    EXPECT_EQ(page.getBackgroundColor(), Colors::white);
}

TEST(PageBackgroundTest, HighlighterOperatorUpdatesOnBackgroundTransitions) {
    XojPage page(595.0, 842.0);
    PageType imageBg(PageTypeFormat::Image);
    PageType pdfBg(PageTypeFormat::Pdf);
    constexpr Color lightBg{0xf1f1f1};
    constexpr Color darkBg{0x1e1e1e};

    // Light Page tests (Multiply):
    //    plain, PDF, plain, Image
    page.setBackgroundColor(lightBg);
    EXPECT_FALSE(page.getBackgroundColor().isDark());
    EXPECT_EQ(page.getBackgroundColor().getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    page.setBackgroundType(pdfBg);
    EXPECT_EQ(page.getBackgroundColor().getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    page.setBackgroundColor(lightBg);
    page.setBackgroundType(imageBg);
    EXPECT_EQ(page.getBackgroundColor().getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    // Dark Page tests (Screen):
    //    plain, PDF, plain, Image
    page.setBackgroundColor(darkBg);
    EXPECT_TRUE(page.getBackgroundColor().isDark());
    EXPECT_EQ(page.getBackgroundColor().getHighlighterOperator(), CAIRO_OPERATOR_SCREEN);

    page.setBackgroundType(pdfBg);
    EXPECT_EQ(page.getBackgroundColor().getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);

    page.setBackgroundColor(darkBg);
    page.setBackgroundType(imageBg);
    EXPECT_EQ(page.getBackgroundColor().getHighlighterOperator(), CAIRO_OPERATOR_MULTIPLY);
}