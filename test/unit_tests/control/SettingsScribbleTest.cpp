#include <fstream>
#include <tuple>

#include <gtest/gtest.h>

#include "control/settings/Settings.h"
#include "util/PathUtil.h"

TEST(SettingsTest, scribbleToEraseDefaultsOffAndRemainsOffForLegacySettings) {
    const fs::path path = Util::getTmpDirSubfolder() / "legacy-scribble-settings.xml";
    {
        std::ofstream file(path);
        file << "<?xml version=\"1.0\"?><settings><property name=\"snapRecognizedShapesEnabled\" "
                "value=\"true\"/></settings>";
    }
    Settings settings(path);
    EXPECT_FALSE(settings.getScribbleToEraseEnabled());
    settings.load();
    EXPECT_FALSE(settings.getScribbleToEraseEnabled());
    EXPECT_TRUE(settings.getSnapRecognizedShapesEnabled());
    fs::remove(path);
}

TEST(SettingsTest, scribbleToErasePersistsEnabledAndDisabledStates) {
    const fs::path path = Util::getTmpDirSubfolder() / "scribble-settings-roundtrip.xml";
    for (bool enabled: {true, false}) {
        Settings settings(path);
        settings.transactionStart();
        settings.setScribbleToEraseEnabled(enabled);
        settings.transactionEnd();
        Settings reloaded(path);
        reloaded.load();
        EXPECT_EQ(reloaded.getScribbleToEraseEnabled(), enabled);
    }
    fs::remove(path);
}

TEST(SettingsTest, scribbleAdvancedOptionsHaveLegacyDefaultsAndRoundTrip) {
    const auto path = Util::getTmpDirSubfolder() / "scribble-advanced-settings.xml";
    {
        std::ofstream file(path);
        file << "<settings/>";
    }
    Settings legacy(path);
    legacy.load();
    EXPECT_EQ(legacy.getScribbleSettings(), (ScribbleSettings{50, 4}));
    for (int sensitivity: {0, 50, 75, 100}) {
        for (int passes: {2, 4, 8}) {
            Settings settings(path);
            settings.transactionStart();
            settings.setScribbleSensitivity(sensitivity);
            settings.setScribbleMinimumPasses(passes);
            settings.transactionEnd();
            Settings reloaded(path);
            reloaded.load();
            EXPECT_EQ(reloaded.getScribbleSettings(), (ScribbleSettings{sensitivity, passes}));
        }
    }
    fs::remove(path);
}

TEST(SettingsTest, scribbleAdvancedOptionsRejectMalformedXmlAndClampRanges) {
    const auto path = Util::getTmpDirSubfolder() / "scribble-invalid-settings.xml";
    for (const char* value: {"", "garbage", "75junk", "50.5", "nan", "999999999999999999999999999"}) {
        {
            std::ofstream file(path);
            file << "<settings><property name=\"scribbleSensitivity\" value=\"" << value
                 << "\"/><property name=\"scribbleMinimumPasses\" value=\"" << value << "\"/></settings>";
        }
        Settings settings(path);
        settings.load();
        EXPECT_EQ(settings.getScribbleSettings(), (ScribbleSettings{50, 4})) << value;
    }
    for (const auto [value, expectedSensitivity, expectedPasses]: {std::tuple{-25, 0, 2}, std::tuple{250, 100, 8}}) {
        {
            std::ofstream file(path);
            file << "<settings><property name=\"scribbleSensitivity\" value=\"" << value
                 << "\"/><property name=\"scribbleMinimumPasses\" value=\"" << value << "\"/></settings>";
        }
        Settings settings(path);
        settings.load();
        EXPECT_EQ(settings.getScribbleSettings(), (ScribbleSettings{expectedSensitivity, expectedPasses}));
        settings.setScribbleSensitivity(value);
        settings.setScribbleMinimumPasses(value);
        EXPECT_EQ(settings.getScribbleSettings(), (ScribbleSettings{expectedSensitivity, expectedPasses}));
    }
    fs::remove(path);
}
