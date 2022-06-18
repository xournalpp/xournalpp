/*
 * Xournal++
 *
 * Configuration of a single plugin
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <gtk/gtk.h>  // for GtkWidget, GtkWindow

#include "gui/GladeGui.h"  // for GladeGui

class Plugin;
class GladeSearchpath;

class PluginDialogEntry: public GladeGui {
public:
    PluginDialogEntry(Plugin* plugin, GladeSearchpath* gladeSearchPath, GtkWidget* w);
    ~PluginDialogEntry() override = default;

public:
    void loadSettings();
    void saveSettings(std::string& pluginEnabled, std::string& pluginDisabled);

    // Not implemented! This is not a dialog!
    void show(GtkWindow* parent) override;

private:
    /**
     * Plugin instance
     */
    Plugin* plugin;
};
