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

#include "config-features.h"

#ifdef ENABLE_PLUGINS

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include "util/raii/GtkWindowUPtr.h"

#include "PluginDialogEntry.h"  // for PluginDialogEntry

class Plugin;
class PluginController;
class Settings;
class GladeSearchpath;

class PluginDialog {
public:
    PluginDialog(GladeSearchpath* gladeSearchPath, Settings* settings,
                 const std::vector<std::unique_ptr<Plugin>>& plugins);

public:
    inline GtkWindow* getWindow() const { return window.get(); }

private:
    void saveSettings();

private:
    Settings* settings;

    std::vector<std::unique_ptr<PluginDialogEntry>> plugins;
    xoj::util::GtkWindowUPtr window;
};

#endif
