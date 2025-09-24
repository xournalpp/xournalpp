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

#include <gtest/gtest.h>

#include "control/settings/Settings.h"

TEST(SettingsTest, testLoadDoesNotThrowForNonExistingFilePath) {
    Settings settings{"non-existing-file-path"};
    EXPECT_NO_THROW(settings.load());
}

// Rudimentary test for Settings save/load - very crude
TEST(SettingsTest, testReadWrite) {
    auto saveReloadTest = [&](const fs::path& dir) {
        std::cout << "Test saving in " << dir.string() << std::endl;
        const fs::path outPath = dir / "xournalpp-test-units_Settings_testReadWrite.xml";
        ASSERT_TRUE(!fs::exists(outPath));

        Settings settings(outPath);
        settings.transactionStart();
        settings.setAudioDisabled(true);                        // bool
        settings.setDefaultSaveName(u8"foo/bar€_%H");           // string
        settings.setDisplayDpi(123);                            // int
        settings.setStabilizerDrag(3.1415);                     // double
        settings.setBackgroundColor(Color(123, 45, 67));        // Color
        settings.setColorPaletteSetting("foo/bar€_palette");    // path
        settings.setEraserVisibility(ERASER_VISIBILITY_HOVER);  // enum
        XojFont testfont("myfontname italic 34");
        settings.setFont(testfont);          // Font
        settings.setPreloadPagesAfter(145);  // unsigned int
        settings.transactionEnd();           // calls save()

        Settings loaded(outPath);
        loaded.load();

        // For each type, we test one that has been changed and one that should be default
        EXPECT_EQ(settings.isAudioDisabled(), loaded.isAudioDisabled());                                  // bool
        EXPECT_EQ(settings.isAutoloadPdfXoj(), loaded.isAutoloadPdfXoj());                                // bool
        EXPECT_EQ(settings.getDefaultSaveName(), loaded.getDefaultSaveName());                            // string
        EXPECT_EQ(settings.getDefaultPdfExportName(), loaded.getDefaultPdfExportName());                  // string
        EXPECT_EQ(settings.getDisplayDpi(), loaded.getDisplayDpi());                                      // int
        EXPECT_EQ(settings.getAddHorizontalSpaceAmountLeft(), loaded.getAddHorizontalSpaceAmountLeft());  // int
        EXPECT_EQ(settings.getStabilizerDrag(), loaded.getStabilizerDrag());                              // double
        EXPECT_EQ(settings.getCursorHighlightBorderWidth(), loaded.getCursorHighlightBorderWidth());      // double
        EXPECT_EQ(settings.getBackgroundColor(), loaded.getBackgroundColor());                            // Color
        EXPECT_EQ(settings.getActiveSelectionColor(), loaded.getActiveSelectionColor());                  // Color
        EXPECT_EQ(settings.getColorPaletteSetting(), loaded.getColorPaletteSetting());                    // path
        EXPECT_EQ(settings.getLastOpenPath(), loaded.getLastOpenPath());                                  // path
        EXPECT_EQ(settings.getEraserVisibility(), loaded.getEraserVisibility());                          // enum
        EXPECT_EQ(settings.getActiveViewMode(), loaded.getActiveViewMode());                              // enum
        EXPECT_EQ(settings.getFont().getName(), loaded.getFont().getName());                              // Font
        EXPECT_EQ(settings.getFont().getSize(), loaded.getFont().getSize());                              // Font
        EXPECT_EQ(settings.getPreloadPagesAfter(), loaded.getPreloadPagesAfter());    // unsigned int
        EXPECT_EQ(settings.getPreloadPagesBefore(), loaded.getPreloadPagesBefore());  // unsigned int

        fs::remove(outPath);
    };
    saveReloadTest(fs::temp_directory_path());
}
