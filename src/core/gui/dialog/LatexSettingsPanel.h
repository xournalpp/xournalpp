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

#include "gui/GladeGui.h"  // for GladeGui

class GladeSearchpath;
class LatexSettings;

class LatexSettingsPanel: public GladeGui {
public:
    LatexSettingsPanel(GladeSearchpath*);
    LatexSettingsPanel(const LatexSettingsPanel&) = delete;
    LatexSettingsPanel& operator=(const LatexSettingsPanel&) = delete;
    LatexSettingsPanel(const LatexSettingsPanel&&) = delete;
    LatexSettingsPanel& operator=(const LatexSettingsPanel&&) = delete;
    ~LatexSettingsPanel() override;

    void show(GtkWindow* parent) override;

    void load(const LatexSettings& settings);
    void save(LatexSettings& settings);

private:
    void checkDeps();

    /**
     * @brief Update whether options accept input ("grayed-out" or not)
     *    based on the current panel state.
     */
    void updateWidgetSensitivity();

    GtkToggleButton* cbAutoDepCheck;
    GtkFileChooser* globalTemplateChooser;
    GtkWidget* sourceViewThemeSelector;
    GtkToggleButton* cbUseSystemFont;
};
