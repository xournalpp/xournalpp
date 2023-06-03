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

#define TEST_GTK(testName) static void testName(GtkApplication* app)

#define RUN_TEST_GTK(testname)                                                \
    auto applicationCallback = +[](GtkApplication* app) {                     \
        testname(app);                                                        \
        g_application_quit(G_APPLICATION(app));                               \
    };                                                                        \
    g_signal_connect(app, "activate", G_CALLBACK(applicationCallback), NULL); \
                                                                              \
    g_application_run(G_APPLICATION(app), argn, argv);


class GtkTest: public ::testing::Test {
protected:
    GtkApplication* app{};
    int argn = 1;
    char** argv{};

    void SetUp() override {
        argn = 1;
        argv = new char*[2];
        argv[0] = "xournalpp_test";
        argv[1] = nullptr;
        app = gtk_application_new("com.github.xournalpp.xournalpp.test", G_APPLICATION_FLAGS_NONE);
    }
};


TEST_GTK(unrenderedPaletteTabShouldYieldNoSelectedPalette) {
    GtkWidget* window = gtk_application_window_new(app);

    GtkGrid* grid = GTK_GRID(gtk_grid_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(grid));

    GtkLabel* mainLabel = GTK_LABEL(gtk_label_new(""));
    GtkListBox* optionsList = GTK_LIST_BOX(gtk_list_box_new());
    gtk_container_add(GTK_CONTAINER(grid), GTK_WIDGET(mainLabel));
    gtk_container_add(GTK_CONTAINER(grid), GTK_WIDGET(optionsList));

    const fs::path palettePath{GET_TESTFILE("palettes/xournalpp.gpl")};

    const std::vector<fs::path> paletteDirectories{palettePath.parent_path()};
    SettingsDialogPaletteTab paletteTab{mainLabel, optionsList, paletteDirectories};
    EXPECT_THROW(paletteTab.getSelectedPalette(), std::runtime_error);
}
TEST_F(GtkTest,
       unrenderedPaletteTabShouldYieldNoSelectedPalette){RUN_TEST_GTK(unrenderedPaletteTabShouldYieldNoSelectedPalette)}


TEST_GTK(renderedPaletteTabShouldNotChangeSelectedColorPalette) {
    GtkWidget* window = gtk_application_window_new(app);

    GtkGrid* grid = GTK_GRID(gtk_grid_new());
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(grid));

    GtkLabel* mainLabel = GTK_LABEL(gtk_label_new(""));
    GtkListBox* optionsList = GTK_LIST_BOX(gtk_list_box_new());
    gtk_container_add(GTK_CONTAINER(grid), GTK_WIDGET(mainLabel));
    gtk_container_add(GTK_CONTAINER(grid), GTK_WIDGET(optionsList));

    const fs::path palettePath{GET_TESTFILE("palettes/xournalpp.gpl")};

    const std::vector<fs::path> paletteDirectories{palettePath.parent_path()};
    SettingsDialogPaletteTab paletteTab{mainLabel, optionsList, paletteDirectories};
    paletteTab.renderPaletteTab(palettePath);
    EXPECT_EQ(palettePath.u8string(), paletteTab.getSelectedPalette().u8string());
}
TEST_F(GtkTest, renderedPaletteTabShouldNotChangeSelectedColorPalette) {
    RUN_TEST_GTK(renderedPaletteTabShouldNotChangeSelectedColorPalette)
}
