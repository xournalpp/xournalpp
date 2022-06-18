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

#include <string>  // for string, basic_string
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkWindow

#include "gui/GladeGui.h"  // for GladeGui

class Settings;
class GladeSearchpath;

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
