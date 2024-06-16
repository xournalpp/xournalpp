#pragma once


#include <filesystem>
#include <optional>
#include <vector>

#include <gtk/gtk.h>

#include "gui/Builder.h"
#include "gui/GladeGui.h"
#include "gui/toolbarMenubar/model/ColorPalette.h"
class SettingsDialogPaletteTab {
public:
    SettingsDialogPaletteTab(GladeSearchpath* gladeSearchPath, const std::vector<fs::path>& paletteDirectories);
    void renderPaletteTab(const fs::path& currentlySetPalettePath);
    auto getSelectedPalette() const -> std::optional<fs::path>;
    inline GtkWidget* getPanel() const { return GTK_WIDGET(panel); }

private:
    std::vector<fs::path> allPaletteFilePaths;
    GtkLabel* colorPaletteExplainLabel;
    GtkListBox* paletteListBox;
    Builder builder;
    GtkScrolledWindow* panel;

    void setAllPaletteFilePaths(const std::vector<fs::path>& paletteDirectories);
    void renderColorPaletteExplainLabel() const;

    static auto newErrorListBoxRow(const fs::path& palettePath, const std::string& error) -> GtkWidget*;
    static auto newPaletteTextBox(const std::string& mainContent, const fs::path& path) -> GtkWidget*;
    static auto newPaletteColorIconsBox(const Palette& palette) -> GtkWidget*;
    static auto newPaletteListBoxRow(Palette& palette) -> GtkWidget*;
    static void renderNoPaletteFoundDisclaimer(GtkListBox* lb);
    static auto renderPaletteListBoxRow(GtkListBox* lb, const fs::path& p) -> GtkWidget*;
};
