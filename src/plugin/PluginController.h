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

#include <string>
#include <vector>

#include "XournalType.h"
#include "filesystem.h"

class Control;
class Plugin;

class PluginController {
public:
    PluginController(Control* control);
    virtual ~PluginController();

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
     * Show Plugin manager Dialog
     */
    void showPluginManager();

    /**
     * Register menu stuff
     */
    void registerMenu();

    /**
     * Return the plugin list
     */
    vector<Plugin*>& getPlugins();

private:
    /**
     * The main controller
     */
    Control* control;

    /**
     * All loaded Plugins
     */
    vector<Plugin*> plugins;
};
