/*
 * Xournal++
 *
 * A single Xournal++ Plugin
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <XournalType.h>

#include <gtk/gtk.h>

#include <config-features.h>

#ifdef ENABLE_PLUGINS
extern "C" {
#include <lua.h>
}

class Plugin;
class Control;

class MenuEntry {
public:
	MenuEntry(Plugin* plugin)
	 : plugin(plugin)
	{
	}

	~MenuEntry()
	{
	}

public:
	/**
	 * The Plugin
	 */
	Plugin* plugin;

	/**
	 * Menu item
	 */
	GtkWidget* widget = nullptr;

	/**
	 * Menu display name
	 */
	string menu;

	/**
	 * Callback function name
	 */
	string callback;

	/**
	 * Accelerator key
	 *
	 * See https://developer.gnome.org/gtk3/stable/gtk3-Keyboard-Accelerators.html#gtk-accelerator-parse
	 */
	string accelerator;
};

class Plugin
{
public:
	Plugin(Control* control, string name, string path);
	virtual ~Plugin();

public:
	/**
	 * Load the plugin script
	 */
	void loadScript();

	/**
	 * Check if this plugin is valid
	 */
	bool isValid();

	/**
	 * Register toolbar item and all other UI stuff
	 */
	void registerToolbar();

	/**
	 * Register all menu entries to the menu
	 */
	void registerMenu(GtkWindow* mainWindow, GtkWidget* menu);

	/**
	 * Execute menu entry
	 */
	void executeMenuEntry(MenuEntry* entry);

	/**
	 * @return the Plugin name
	 */
	string getName();

	/**
	 * @return Description of the plugin
	 */
	string getDescription();

	/**
	 * Author of the plugin
	 */
	string getAuthor();

	/**
	 * Plugin version
	 */
	string getVersion();

	/**
	 * The plugin is enabled
	 */
	bool isEnabled();

	/**
	 * The plugin is enabled
	 */
	void setEnabled(bool enabled);

	/**
	 * The plugin is default enabled
	 */
	bool isDefaultEnabled();

	/**
	 * @return Flag to check if init ui is currently running
	 */
	bool isInInitUi();

	/**
	 * Register a menu item
	 *
	 * @return Internal ID, can e.g. be used to disable the menu
	 */
	int registerMenu(string menu, string callback, string accelerator);

	/**
	 * @return The main controller
	 */
	Control* getControl();

private:
	/**
	 * Load ini file
	 */
	void loadIni();

	/**
	 * Execute lua function
	 */
	bool callFunction(string fnc);

	/**
	 * Load custom Lua Libraries
	 */
	void registerXournalppLibs(lua_State* lua);

	/**
	 * Add the plugin folder to the lua path
	 */
	void addPluginToLuaPath();

public:
	/**
	 * Get Plugin from lua engine
	 */
	static Plugin* getPluginFromLua(lua_State* lua);

private:
	/**
	 * Plugin root path
	 */
	string path;

	/**
	 * Plugin name
	 */
	string name;

	/**
	 * Description of the plugin
	 */
	string description;

	/**
	 * Author of the plugin
	 */
	string author;

	/**
	 * Plugin version
	 */
	string version;

	/**
	 * Main plugin script
	 */
	string mainfile;

	/**
	 * The plugin is enabled
	 */
	bool enabled = false;

	/**
	 * The plugin is default enabled
	 */
	bool defaultEnabled = false;

	/**
	 * Lua engine
	 */
	lua_State* lua = nullptr;

	/**
	 * All registered menu entries
	 */
	vector<MenuEntry*> menuEntries;

	/**
	 * The main controller
	 */
	Control* control;

	/**
	 * Flag to check if init ui is currently running
	 */
	bool inInitUi = false;

	/**
	 * Flag if the plugin is valid / correct loaded
	 */
	bool valid = false;
};

#endif

