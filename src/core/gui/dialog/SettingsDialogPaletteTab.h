#pragma once


#include <filesystem>
#include <vector>

#include <gtk/gtk.h>

#include "gui/GladeGui.h"
#include "gui/toolbarMenubar/model/ColorPalette.h"
class SettingsDialogPaletteTab {
public:
    SettingsDialogPaletteTab(GtkLabel* colorPaletteExplainLabel, GtkListBox* paletteListBox,
                             const std::vector<fs::path>& paletteDirectories);
    void renderPaletteTab(const fs::path& currentlySetPalettePath);
    auto getSelectedPalette() -> fs::path;

private:
    std::vector<fs::path> allPaletteFilePaths;
    GtkLabel* colorPaletteExplainLabel;
    GtkListBox* paletteListBox;

    void setAllPaletteFilePaths(const std::vector<fs::path>& paletteDirectories);
    void renderColorPaletteExplainLabel() const;

    static auto newErrorListBoxRow(const fs::path& palettePath, const std::string& error) -> GtkWidget*;
    static auto newPaletteTextBox(const std::string& mainContent, const fs::path& path) -> GtkWidget*;
    static auto newPaletteColorIconsBox(const Palette& palette) -> GtkWidget*;
    static auto newPaletteListBoxRow(Palette& palette) -> GtkWidget*;
    static void renderNoPaletteFoundDisclaimer(GtkListBox* lb) ;
    static auto renderPaletteListBoxRow(GtkListBox* lb, const fs::path& p) -> GtkWidget* ;
};
