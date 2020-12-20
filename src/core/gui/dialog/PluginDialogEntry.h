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

#include "gui/GladeGui.h"

class Settings;
class Plugin;
class PluginDialog;

class PluginDialogEntry: public GladeGui {
public:
    PluginDialogEntry(Plugin* plugin, GladeSearchpath* gladeSearchPath, GtkWidget* w);
    ~PluginDialogEntry() override = default;

public:
    void loadSettings();
    void saveSettings(string& pluginEnabled, string& pluginDisabled);

    // Not implemented! This is not a dialog!
    void show(GtkWindow* parent) override;

private:
    /**
     * Plugin instance
     */
    Plugin* plugin;
};
