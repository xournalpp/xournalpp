/*
 * Xournal++
 *
 * Plugin manage dialog
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include <gtk/gtk.h>  // for GtkWindow

#include "gui/GladeGui.h"  // for GladeGui

#include "PluginDialogEntry.h"  // for PluginDialogEntry

class PluginController;
class Settings;
class GladeSearchpath;

class PluginDialog: public GladeGui {
public:
    PluginDialog(GladeSearchpath* gladeSearchPath, Settings* settings);
    ~PluginDialog() override = default;

public:
    void loadPluginList(PluginController const* pc);
    void show(GtkWindow* parent) override;

private:
    void saveSettings();

private:
    Settings* settings;

    std::vector<std::unique_ptr<PluginDialogEntry>> plugins;
};
