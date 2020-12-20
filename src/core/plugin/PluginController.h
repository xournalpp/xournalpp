/*
 * Xournal++
 *
 * Plugin main controller
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Plugin.h"
#include "filesystem.h"

class Control;

class PluginController final {
public:
    explicit PluginController(Control* control);

public:
    /**
     * Load all plugins within this folder
     *
     * @param path The path which contains the plugin folders
     */
    void loadPluginsFrom(fs::path const& path);

    /**
     * Register toolbar item and all other UI stuff
     */
    void registerToolbar();

    /**
     * Register menu stuff
     */
    void registerMenu();

    /**
     * Show Plugin manager Dialog
     */
    void showPluginManager() const;

    /**
     * Return the plugin list
     */
    auto getPlugins() const -> std::vector<Plugin*>;

private:
    /**
     * The main controller
     */
    Control* control;

    /**
     * All loaded Plugins
     */
    std::vector<std::unique_ptr<Plugin>> plugins;
};
