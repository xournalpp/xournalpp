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