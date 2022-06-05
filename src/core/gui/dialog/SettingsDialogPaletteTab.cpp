#include "SettingsDialogPaletteTab.h"

#include "gui/GladeGui.h"
#include "gui/toolbarMenubar/icon/ColorSelectImage.h"
#include "util/PathUtil.h"
#include "util/i18n.h"

static const char* const G_OBJECT_PALETTE_PATH = "xournalpp.palettePath";


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HELPER
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

auto concatenated(const std::vector<fs::path>& p1, const std::vector<fs::path>& p2) -> std::vector<fs::path> {
    std::vector<fs::path> result{};
    result.insert(result.end(), p1.begin(), p1.end());
    result.insert(result.end(), p2.begin(), p2.end());
    return result;
}

std::string colorize(const std::string& text, const std::string& color) {
    return std::string{"<span foreground=\""} + color + std::string{"\">"} + text + std::string{"</span>"};
}

std::string pathLink(const fs::path& path) { return FS(_F("<a href=\"file://{1}\">{1}</a>") % path.u8string()); }

void setGObjectPalettePath(GObject* gObject, const fs::path& path) {
    g_object_set_data(gObject, G_OBJECT_PALETTE_PATH, (gpointer)&path);
}

fs::path getGObjectPalettePath(GObject* gObject) {
    auto* pathPointer = (fs::path*)g_object_get_data(gObject, G_OBJECT_PALETTE_PATH);
    return *pathPointer;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public Methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SettingsDialogPaletteTab::SettingsDialogPaletteTab(GtkLabel* colorPaletteExplainLabel, GtkListBox* paletteListBox,
                                                   const std::vector<fs::path>& paletteDirectories):
        currentlySetPalettePath{currentlySetPalettePath},
        colorPaletteExplainLabel{colorPaletteExplainLabel},
        paletteListBox{paletteListBox} {
    renderColorPaletteExplainLabel();
    setAllPaletteFilePaths(paletteDirectories);
}

void SettingsDialogPaletteTab::renderPaletteTab(const fs::path& currentlySetPalettePath) {
    GtkListBox* lb = paletteListBox;
    if (allPaletteFilePaths.empty()) {
        renderNoPaletteFoundDisclaimer(lb);
        return;
    }

    for (const fs::path& p: allPaletteFilePaths) {
        GtkWidget* listBoxRow = renderPaletteListBoxRow(lb, p);

        if (p == currentlySetPalettePath)
            gtk_list_box_select_row(GTK_LIST_BOX(lb), GTK_LIST_BOX_ROW(listBoxRow));
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GtkWidget* SettingsDialogPaletteTab::renderPaletteListBoxRow(GtkListBox* lb, const fs::path& p) const {
    Palette palette{p};
    GtkWidget* listBoxRow = nullptr;

    try {
        palette.load();
        listBoxRow = newPaletteListBoxRow(palette);
    } catch (const std::exception& e) {
        listBoxRow = newErrorListBoxRow(p, e.what());
    }
    setGObjectPalettePath(G_OBJECT(listBoxRow), p);

    gtk_list_box_prepend(lb, listBoxRow);
    return listBoxRow;
}

void SettingsDialogPaletteTab::renderColorPaletteExplainLabel() const {
    gtk_label_set_label(colorPaletteExplainLabel,
                        FS(_F("<i>The palettes shown below are obtained from:\n"
                              "   - Built-in palettes: {1}\n"
                              "   - User palettes: {2}.\n</i>") %
                           pathLink(Util::getPalettePath()) % pathLink(Util::getConfigFile("palettes")))
                                .c_str());
    gtk_label_set_use_markup(colorPaletteExplainLabel, true);
}

void SettingsDialogPaletteTab::renderNoPaletteFoundDisclaimer(GtkListBox* lb) const {
    GtkWidget* label = gtk_label_new("<span foreground=\"red\">"
                                     "No palette files (i.e. with extension .gpl) could be found.\n"
                                     "Using the default until another palette is configured."
                                     "</span>");
    gtk_label_set_use_markup(GTK_LABEL(label), true);
    gtk_widget_show(label);
    gtk_list_box_prepend(lb, label);
}

// use list of fs::path as input
void SettingsDialogPaletteTab::setAllPaletteFilePaths(const std::vector<fs::path>& paletteDirectories) {
    for (const fs::path& paletteDirectory: paletteDirectories) {
        std::vector<fs::path> const files = Util::listFilesSorted(paletteDirectory);
        for (const fs::path& paletteFile: files) {
            if (paletteFile.extension() == ".gpl") {
                allPaletteFilePaths.push_back(paletteFile);
            }
        }
    }
}

auto SettingsDialogPaletteTab::getSelectedPalette() -> fs::path {
    GtkListBoxRow* selected_listbox_row = gtk_list_box_get_selected_row(paletteListBox);
    if (G_IS_OBJECT(selected_listbox_row)) {
        return getGObjectPalettePath(G_OBJECT(selected_listbox_row));
    } else {
        throw std::runtime_error("The SettingsDialog must be rendered before trying to obtain the selected palette.");
    }
}

auto SettingsDialogPaletteTab::newErrorListBoxRow(const fs::path& palettePath, const std::string& error) -> GtkWidget* {
    GtkWidget* listBoxRow = gtk_list_box_row_new();
    GtkWidget* rowContent = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_container_add(GTK_CONTAINER(listBoxRow), rowContent);

    std::string formattedError = colorize("Error: " + error, "red");
    GtkWidget* text = newPaletteTextBox(formattedError, palettePath.u8string());
    gtk_box_pack_start(GTK_BOX(rowContent), text, false, false, 0);

    gtk_widget_show(rowContent);
    gtk_widget_show(listBoxRow);

    return listBoxRow;
}

auto SettingsDialogPaletteTab::newPaletteListBoxRow(Palette& palette) -> GtkWidget* {
    GtkWidget* listBoxRow = gtk_list_box_row_new();
    GtkWidget* rowContent = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    gtk_container_add(GTK_CONTAINER(listBoxRow), rowContent);

    std::string paletteName = palette.getHeader(std::string{"Name"});
    GtkWidget* text = nullptr;
    if (paletteName.empty())
        text = newPaletteTextBox(std::string{"<i>Palette has no Name</i>"}, palette.getFilePath().u8string());
    else
        text = newPaletteTextBox(paletteName, palette.getFilePath().u8string());
    gtk_box_pack_start(GTK_BOX(rowContent), text, false, false, 0);

    GtkWidget* colorIcons = newPaletteColorIconsBox(palette);
    gtk_box_pack_start(GTK_BOX(rowContent), colorIcons, true, true, 0);

    gtk_widget_show(rowContent);
    gtk_widget_show(listBoxRow);

    return listBoxRow;
}

auto SettingsDialogPaletteTab::newPaletteTextBox(const std::string& mainContent, const std::string& additionalInfo)
        -> GtkWidget* {
    GtkWidget* textBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    gtk_widget_show(textBox);

    GtkWidget* mainLabel = gtk_label_new(mainContent.c_str());
    gtk_widget_set_halign(mainLabel, GTK_ALIGN_START);
    gtk_label_set_use_markup(GTK_LABEL(mainLabel), true);
    gtk_box_pack_start(GTK_BOX(textBox), mainLabel, false, false, 0);
    gtk_widget_show(mainLabel);

    std::string secondaryInformation = std::string{"└─ "} + additionalInfo;
    GtkWidget* secondaryLabel = gtk_label_new(colorize(secondaryInformation, "gray").c_str());
    gtk_widget_set_halign(secondaryLabel, GTK_ALIGN_START);
    gtk_label_set_use_markup(GTK_LABEL(secondaryLabel), true);
    gtk_box_pack_start(GTK_BOX(textBox), secondaryLabel, false, false, 0);
    gtk_widget_show(secondaryLabel);

    return textBox;
}

auto SettingsDialogPaletteTab::newPaletteColorIconsBox(const Palette& palette) -> GtkWidget* {
    GtkWidget* colors = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
    for (unsigned long i = 0; i < palette.size(); i++) {
        const NamedColor& namedColor = palette.getColorAt(i);
        const Color c = namedColor.getColor();
        GtkWidget* icon = ColorSelectImage::newColorIcon(c, 16, true);
        gtk_widget_show(icon);
        gtk_box_pack_start(GTK_BOX(colors), icon, false, false, 0);
    }
    gtk_widget_show(colors);
    gtk_widget_set_halign(colors, GTK_ALIGN_END);
    return colors;
}
