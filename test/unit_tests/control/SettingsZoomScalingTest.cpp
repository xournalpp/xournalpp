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

#include "control/ToolHandler.h"
#include "control/settings/Settings.h"
#include "filesystem.h"

class DummyListener: public ToolListener {
public:
    void toolColorChanged() override {}
    void changeColorOfSelection() override {}
    void toolSizeChanged() override {}
    void toolFillChanged() override {}
    void toolLineStyleChanged() override {}
    void toolChanged() override {}
};

TEST(SettingsZoomScalingTest, SaveAndLoadToolZoomScaling) {
    fs::path tempSettings = fs::temp_directory_path() / "test_settings_zoom.xml";
    if (fs::exists(tempSettings)) {
        fs::remove(tempSettings);
    }

    DummyListener listener;
    Settings settings(tempSettings);
    ToolHandler handler(&listener, nullptr, &settings);

    // Enable zoom scaling on eraser and highlighter, disable on pen
    handler.getTool(TOOL_ERASER).setZoomScaling(true);
    handler.getTool(TOOL_PEN).setZoomScaling(false);
    handler.getTool(TOOL_HIGHLIGHTER).setZoomScaling(true);

    handler.saveSettings();
    settings.save();

    // Reload into a fresh handler
    Settings settingsReload(tempSettings);
    settingsReload.load();
    ToolHandler handlerReload(&listener, nullptr, &settingsReload);
    handlerReload.loadSettings();

    EXPECT_TRUE(handlerReload.getTool(TOOL_ERASER).isZoomScaling());
    EXPECT_FALSE(handlerReload.getTool(TOOL_PEN).isZoomScaling());
    EXPECT_TRUE(handlerReload.getTool(TOOL_HIGHLIGHTER).isZoomScaling());

    if (fs::exists(tempSettings)) {
        fs::remove(tempSettings);
    }
}

TEST(SettingsZoomScalingTest, MissingSettingDefaultsToFalse) {
    fs::path tempSettings = fs::temp_directory_path() / "test_settings_zoom_defaults.xml";
    if (fs::exists(tempSettings)) {
        fs::remove(tempSettings);
    }

    DummyListener listener;
    Settings settings(tempSettings);
    ToolHandler handler(&listener, nullptr, &settings);

    // Save default settings without modifying zoom scaling
    handler.saveSettings();
    settings.save();

    Settings settingsReload(tempSettings);
    settingsReload.load();
    ToolHandler handlerReload(&listener, nullptr, &settingsReload);
    handlerReload.loadSettings();

    EXPECT_FALSE(handlerReload.getTool(TOOL_PEN).isZoomScaling());
    EXPECT_FALSE(handlerReload.getTool(TOOL_ERASER).isZoomScaling());
    EXPECT_FALSE(handlerReload.getTool(TOOL_HIGHLIGHTER).isZoomScaling());

    if (fs::exists(tempSettings)) {
        fs::remove(tempSettings);
    }
}
