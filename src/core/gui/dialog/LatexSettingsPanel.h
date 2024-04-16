/*
 * Xournal++
 *
 * Latex settings panel
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <gtk/gtk.h>  // for GtkToggleButton, GtkFileChooser, GtkWidget

#include "gui/Builder.h"

#include "filesystem.h"

class GladeSearchpath;
class LatexSettings;

class LatexSettingsPanel {
public:
    explicit LatexSettingsPanel(GladeSearchpath*);
    LatexSettingsPanel(const LatexSettingsPanel&) = delete;
    LatexSettingsPanel& operator=(const LatexSettingsPanel&) = delete;
    LatexSettingsPanel(const LatexSettingsPanel&&) = delete;
    LatexSettingsPanel& operator=(const LatexSettingsPanel&&) = delete;

    void load(const LatexSettings& settings);
    void save(LatexSettings& settings);

    inline GtkWidget* getPanel() const { return GTK_WIDGET(panel); }

private:
    void checkDeps();

    /**
     * @brief Update whether options accept input ("grayed-out" or not)
     *    based on the current panel state.
     */
    void updateWidgetSensitivity();

    void setTemplateFile(fs::path p);

    Builder builder;
    GtkScrolledWindow* panel;

    GtkCheckButton* cbAutoDepCheck;
    GtkWidget* sourceViewThemeSelector;
    GtkCheckButton* cbUseSystemFont;

    GtkButton* templateFileButton;
    fs::path latexTemplateFile;
};
