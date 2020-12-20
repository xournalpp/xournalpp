/*
 * Xournal++
 *
 * The configuration for a language in the Settings Dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <vector>

#include <gdk/gdk.h>

#include "gui/GladeGui.h"

class Settings;
class SettingsDialog;

class LanguageConfigGui: public GladeGui {
public:
    LanguageConfigGui(GladeSearchpath* gladeSearchPath, GtkWidget* w, Settings* settings);

public:
    void loadSettings(){};
    void saveSettings();

    // Not implemented! This is not a dialog!
    void show(GtkWindow* parent) override{};

private:
    std::vector<std::string> availableLocales;


    Settings* settings;
};
