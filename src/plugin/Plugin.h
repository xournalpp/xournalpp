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

#include <config-features.h>

#ifdef ENABLE_PLUGINS
#include <lua.h>


class Plugin
{
public:
	Plugin(string name, string path);
	virtual ~Plugin();

public:
	/**
	 * Check if this plugin is valid
	 */
	bool isValid();

	/**
	 * Register toolbar item and all other UI stuff
	 */
	void registerToolbar();

	/**
	 * @return the Plugin name
	 */
	string getName();

	/**
	 * @return Flag to check if init ui is currently running
	 */
	bool isInInitUi();

	/**
	 * Register a menu item
	 *
	 * @return Internal ID, can e.g. be used to disable the menu
	 */
	int registerMenu(string menu, string callback);

private:
	/**
	 * Load ini file
	 */
	void loadIni();

	/**
	 * Load the plugin script
	 */
	void loadScript();

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
	XOJ_TYPE_ATTRIB;

	/**
	 * Plugin root path
	 */
	string path;

	/**
	 * Plugin name
	 */
	string name;

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
	 * Lua engine
	 */
	lua_State* lua = NULL;

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

