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

#include "control/Control.h"
#include "control/XournalMain.h"
#include "control/settings/Settings.h"
#include "gui/GladeSearchpath.h"
#include "gui/MainWindow.h"
#include "gui/dialog/SettingsDialog.h"
#include "util/PathUtil.h"
#include "util/Stacktrace.h"

#include "GtkTest.h"
#include "config-dev.h"
#include "config-test.h"

class ShowAndSaveDoesNotThrow: public GtkTest {
    void runTest(GtkApplication* app) override {
        GladeSearchpath gladeSearchpath{};
        XournalMain::initResourcePath(&gladeSearchpath, "ui/about.glade", true);
        XournalMain::initResourcePath(&gladeSearchpath, "ui/xournalpp.css", true);

        GApplication* gApp = G_APPLICATION(app);

        Control control{gApp, &gladeSearchpath};

        // If initializing differently, i.e.:
        // MainWindow mainWindow{MainWindow(&gladeSearchpath, &control, app)};
        // the test fails at the end of the block with a segfault
        MainWindow* mainWindow = new MainWindow(&gladeSearchpath, &control, app);
        control.initWindow(mainWindow);

        Settings settings{Util::getConfigFile(SETTINGS_XML_FILE)};
        settings.load();

        const fs::path palettePath{GET_TESTFILE("palettes/xournalpp.gpl")};
        const std::vector<fs::path> paletteDirectories{palettePath.parent_path()};

        SettingsDialog settingsDialog{&gladeSearchpath, &settings, &control, paletteDirectories};

        settingsDialog.load();
        g_message("Loaded settings\n");
        EXPECT_NO_THROW(settingsDialog.save());
    }
};
TEST_F(ShowAndSaveDoesNotThrow, showAndSafeDoesNotThrow) {}

class ShowAndSaveDoesNotThrowForNoPalettes: public GtkTest {
    void runTest(GtkApplication* app) override {
        GladeSearchpath gladeSearchpath{};
        XournalMain::initResourcePath(&gladeSearchpath, "ui/about.glade", true);
        XournalMain::initResourcePath(&gladeSearchpath, "ui/xournalpp.css", true);

        GApplication* gApp = G_APPLICATION(app);

        Control control{gApp, &gladeSearchpath};

        // If initializing differently, i.e.:
        // MainWindow mainWindow{MainWindow(&gladeSearchpath, &control, app)};
        // the test fails at the end of the block with a segfault
        MainWindow* mainWindow = new MainWindow(&gladeSearchpath, &control, app);
        control.initWindow(mainWindow);

        Settings settings{Util::getConfigFile(SETTINGS_XML_FILE)};
        settings.load();

        const fs::path emptyFile{GET_TESTFILE("no_palettes/empty.txt")};
        const std::vector<fs::path> paletteDirectories{emptyFile.parent_path()};

        SettingsDialog settingsDialog{&gladeSearchpath, &settings, &control, paletteDirectories};

        settingsDialog.load();
        g_message("Loaded settings\n");
        EXPECT_NO_THROW(settingsDialog.save());
    }
};
TEST_F(ShowAndSaveDoesNotThrowForNoPalettes, ShowAndSaveDoesNotThrowForNoPalettes) {}
