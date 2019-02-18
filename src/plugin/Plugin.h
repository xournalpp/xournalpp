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
	 * Flag if the plugin is valid / correct loaded
	 */
	bool valid = false;
};

#endif

