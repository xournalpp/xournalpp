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

#include "gui/dialog/SettingsDialogPaletteTab.h"

#include "config-test.h"

class GtkTest: public ::testing::Test {
protected:
    GtkApplication* app{};
    int argn = 1;
    char** argv{};
    GtkGrid* grid = nullptr;
    GtkLabel* mainLabel = nullptr;
    GtkListBox* optionsList = nullptr;

    virtual void runTest(GtkApplication* app) = 0;

    // Setting up the testing environment
    void SetUp() override {
        argn = 1;
        argv = new char*[2];
        argv[0] = "xournalpp_test";
        argv[1] = nullptr;
        app = gtk_application_new("com.github.xournalpp.xournalpp.test", G_APPLICATION_FLAGS_NONE);
        g_signal_connect(app, "activate", G_CALLBACK(applicationCallback), this);
        g_application_run(G_APPLICATION(app), argn, argv);
    }

    // This the callback in which the actual test is run
    // It needs to be a callback because it requires the GtkApplication to be running already.
    static void applicationCallback(GtkApplication* app, gpointer userData) {
        auto* test = static_cast<GtkTest*>(userData);

        // setup the Widget to be tested
        createAndSetupWidgets(app, &test->grid, &test->mainLabel, &test->optionsList);

        // run the actual test
        test->runTest(app);

        // Quit the application to avoid waiting indefinitely fo the test to finish
        g_application_quit(G_APPLICATION(app));
    }

    // Helper method to create and setup widgets
    static void createAndSetupWidgets(GtkApplication* app, GtkGrid** grid, GtkLabel** mainLabel,
                                      GtkListBox** optionsList) {
        GtkWidget* window = gtk_application_window_new(app);

        *grid = GTK_GRID(gtk_grid_new());
        gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(*grid));

        *mainLabel = GTK_LABEL(gtk_label_new(""));
        *optionsList = GTK_LIST_BOX(gtk_list_box_new());
        gtk_container_add(GTK_CONTAINER(*grid), GTK_WIDGET(*mainLabel));
        gtk_container_add(GTK_CONTAINER(*grid), GTK_WIDGET(*optionsList));
    }
};


class UnrenderedPaletteTabTest: public GtkTest {
    void runTest(GtkApplication* app) override {
        const fs::path palettePath{GET_TESTFILE("palettes/xournalpp.gpl")};

        const std::vector<fs::path> paletteDirectories{palettePath.parent_path()};
        SettingsDialogPaletteTab paletteTab{mainLabel, optionsList, paletteDirectories};
        EXPECT_THROW(paletteTab.getSelectedPalette(), std::runtime_error);
    }
};
TEST_F(UnrenderedPaletteTabTest, unrenderedPaletteTabShouldYieldNoSelectedPalette) {}

class RenderedPaletteTabTest: public GtkTest {
    void runTest(GtkApplication* app) override {
        const fs::path palettePath{GET_TESTFILE("palettes/xournalpp.gpl")};

        const std::vector<fs::path> paletteDirectories{palettePath.parent_path()};
        SettingsDialogPaletteTab paletteTab{mainLabel, optionsList, paletteDirectories};
        paletteTab.renderPaletteTab(palettePath);
        EXPECT_EQ(palettePath.u8string(), paletteTab.getSelectedPalette().u8string());
    }
};
TEST_F(RenderedPaletteTabTest, renderedPaletteTabShouldNotChangeSelectedColorPalette) {}
