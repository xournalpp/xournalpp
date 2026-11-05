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
#include "control/settings/ButtonConfig.h"
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

TEST(ButtonConfigZoomScalingTest, InheritsWhenDontChange) {
    DummyListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);

    handler.getTool(TOOL_ERASER).setZoomScaling(true);

    ButtonConfig cfg(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    cfg.setZoomScalingMode(ZoomScalingMode::DONT_CHANGE);
    cfg.initButton(&handler, Button::BUTTON_ERASER);

    EXPECT_TRUE(handler.getButtonTool(Button::BUTTON_ERASER)->isZoomScaling());

    handler.getTool(TOOL_ERASER).setZoomScaling(false);
    cfg.initButton(&handler, Button::BUTTON_ERASER);
    EXPECT_FALSE(handler.getButtonTool(Button::BUTTON_ERASER)->isZoomScaling());
}

TEST(ButtonConfigZoomScalingTest, OverridesWhenEnabledOrDisabled) {
    DummyListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);

    handler.getTool(TOOL_ERASER).setZoomScaling(false);

    ButtonConfig cfgEnabled(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    cfgEnabled.setZoomScalingMode(ZoomScalingMode::ENABLED);
    cfgEnabled.initButton(&handler, Button::BUTTON_ERASER);
    EXPECT_TRUE(handler.getButtonTool(Button::BUTTON_ERASER)->isZoomScaling());

    ButtonConfig cfgDisabled(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    cfgDisabled.setZoomScalingMode(ZoomScalingMode::DISABLED);
    cfgDisabled.initButton(&handler, Button::BUTTON_ERASER);
    EXPECT_FALSE(handler.getButtonTool(Button::BUTTON_ERASER)->isZoomScaling());
}

TEST(ButtonConfigZoomScalingTest, ApplyNoChangeSettingsInheritsZoomScaling) {
    DummyListener listener;
    ToolHandler handler(&listener, nullptr, nullptr);

    handler.getTool(TOOL_ERASER).setZoomScaling(false);

    ButtonConfig cfg(TOOL_ERASER, Colors::black, TOOL_SIZE_NONE, DRAWING_TYPE_DEFAULT, ERASER_TYPE_NONE);
    cfg.setZoomScalingMode(ZoomScalingMode::DONT_CHANGE);
    cfg.initButton(&handler, Button::BUTTON_ERASER);
    EXPECT_FALSE(handler.getButtonTool(Button::BUTTON_ERASER)->isZoomScaling());

    // Primary tool changes dynamically
    handler.getTool(TOOL_ERASER).setZoomScaling(true);
    cfg.applyNoChangeSettings(&handler, Button::BUTTON_ERASER);
    EXPECT_TRUE(handler.getButtonTool(Button::BUTTON_ERASER)->isZoomScaling());
}

TEST(ButtonConfigZoomScalingTest, SaveAndLoadButtonConfigZoomScalingMode) {
    fs::path tempSettings = fs::temp_directory_path() / "test_settings_button_zoom.xml";
    if (fs::exists(tempSettings)) {
        fs::remove(tempSettings);
    }

    {
        Settings settings(tempSettings);
        ButtonConfig* eraserCfg = settings.getButtonConfig(Button::BUTTON_ERASER);
        eraserCfg->setZoomScalingMode(ZoomScalingMode::ENABLED);

        ButtonConfig* stylus1Cfg = settings.getButtonConfig(Button::BUTTON_STYLUS_ONE);
        stylus1Cfg->setZoomScalingMode(ZoomScalingMode::DISABLED);

        ButtonConfig* stylus2Cfg = settings.getButtonConfig(Button::BUTTON_STYLUS_TWO);
        stylus2Cfg->setZoomScalingMode(ZoomScalingMode::DONT_CHANGE);

        settings.save();
    }

    {
        Settings settingsReload(tempSettings);
        settingsReload.load();

        EXPECT_EQ(settingsReload.getButtonConfig(Button::BUTTON_ERASER)->getZoomScalingMode(),
                  ZoomScalingMode::ENABLED);
        EXPECT_EQ(settingsReload.getButtonConfig(Button::BUTTON_STYLUS_ONE)->getZoomScalingMode(),
                  ZoomScalingMode::DISABLED);
        EXPECT_EQ(settingsReload.getButtonConfig(Button::BUTTON_STYLUS_TWO)->getZoomScalingMode(),
                  ZoomScalingMode::DONT_CHANGE);
    }

    if (fs::exists(tempSettings)) {
        fs::remove(tempSettings);
    }
}
