#pragma once


#include <filesystem>
#include <vector>

#include <gtk/gtk.h>

#include "gui/GladeGui.h"
#include "gui/toolbarMenubar/model/ColorPalette.h"
class SettingsDialogPaletteTab {
public:
    SettingsDialogPaletteTab(GtkLabel* colorPaletteExplainLabel,
                             GtkListBox* paletteListBox);
    void renderPaletteTab(const fs::path& currentlySetPalettePath);
    fs::path getSelectedPalette();

private:
    std::vector<fs::path> allPaletteFilePaths;
    const fs::path& currentlySetPalettePath;
    GtkLabel* colorPaletteExplainLabel;
    GtkListBox* paletteListBox;

    static GtkWidget* newErrorListBoxRow(const fs::path& palettePath, const std::string& error) ;
    static GtkWidget* newPaletteTextBox(const std::string& mainContent, const std::string& additionalInfo) ;
    static GtkWidget* newPaletteColorIconsBox(const Palette& palette) ;
    static GtkWidget* newPaletteListBoxRow(Palette& palette) ;
    void setAllPaletteFilePaths();
    void renderNoPaletteFoundDisclaimer(GtkListBox* lb) const;
    void renderColorPaletteExplainLabel() const;
    GtkWidget* renderPaletteListBoxRow(GtkListBox* lb, const fs::path& p) const;
};
