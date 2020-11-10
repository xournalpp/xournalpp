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

#include <memory>

#include "gui/GladeGui.h"

#include "PluginDialogEntry.h"

class PluginController;
class Settings;

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
