#include "Plugin.h"

#include <config.h>

#ifdef ENABLE_PLUGINS

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define LOAD_FROM_INI(target, group, key) \
	{ \
		char* value = g_key_file_get_string(config, group, key, NULL); \
		if (value != NULL) \
		{ \
			target = value; \
			g_free(value); \
		} \
	}


Plugin::Plugin(string name, string path)
 : name(name),
   path(path)
{
	XOJ_INIT_TYPE(Plugin);

	loadIni();
	loadScript();
}

Plugin::~Plugin()
{
	XOJ_CHECK_TYPE(Plugin);

	if (lua)
	{
		// Clean up, free the Lua state var
		lua_close(lua);
		lua = NULL;
	}

	XOJ_RELEASE_TYPE(Plugin);
}

Plgin* getPluginFromLua(lua_State* lua)
{
	lua_getfield(lua, LUA_REGISTRYINDEX, "Xournalpp_Plugin");

	if (lua_islightuserdata(luaState, -1))
	{
		lua_
		lua_pop(luaState, 1);
	}

	return NULL;
}

/**
 * Register toolbar item and all other UI stuff
 */
void Plugin::registerToolbar()
{
	XOJ_CHECK_TYPE(Plugin);

	if (!this->valid)
	{
		return;
	}

	// lua_register
	// lua_atpanic
	// luaL_ref LUA_REGISTRYINDEX

	if (callFunction("initUi"))
	{
		g_message("Plugin «%s» UI initialized", name.c_str());
	}
	else
	{
		g_message("Plugin «%s» has no UI init", name.c_str());
	}
}

/**
 * Load ini file
 */
void Plugin::loadIni()
{
	XOJ_CHECK_TYPE(Plugin);

	GKeyFile* config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');

	string filename = path + "/plugin.ini";
	if (!g_key_file_load_from_file(config, filename.c_str(), G_KEY_FILE_NONE, NULL))
	{
		g_key_file_free(config);
		return;
	}

	LOAD_FROM_INI(author, "about", "author");
	LOAD_FROM_INI(version, "about", "version");

	if (version == "<xournalpp>")
	{
		version = PROJECT_VERSION;
	}

	LOAD_FROM_INI(mainfile, "plugin", "mainfile");

	g_key_file_free(config);

	this->valid = true;
}

/**
 * Load the plugin script
 */
void Plugin::loadScript()
{
	XOJ_CHECK_TYPE(Plugin);

	if (mainfile == "")
	{
		this->valid = false;
		return;
	}

	if (mainfile.find("..") != std::string::npos)
	{
		g_warning("Plugin «%s» contains unsupported path «%s»", name.c_str(), mainfile.c_str());
		this->valid = false;
		return;
	}

	// Create Lua state variable
    lua = luaL_newstate();

    // Load Lua libraries
    luaL_openlibs(lua);

    // Load but don't run the Lua script
    string luafile = path + "/" + mainfile;
	if (luaL_loadfile(lua, luafile.c_str()))
	{
		// Error out if file can't be read
		g_warning("Could not run plugin Lua file: «%s»", luafile.c_str());
		this->valid = false;
		return;
	}

	// Register Plugin object to Lua instance
	// lua_newuserdata
	lua_pushlightuserdata(lua, this);
	lua_setfield(lua, LUA_REGISTRYINDEX, "Xournalpp_Plugin");


	// Run the loaded Lua script
	if (lua_pcall(lua, 0, 0, 0))
	{
		g_warning("Could not run plugin Lua file: «%s», error.", luafile.c_str());
		this->valid = false;
		return;
	}
}

/**
 * Execute lua function
 */
bool Plugin::callFunction(string fnc)
{
	lua_getglobal(lua, fnc.c_str());

	// Run the function
	if (lua_pcall(lua, 0, 0, 0))
	{
		// Failed
		return false;
	}

	return true;
}

/**
 * Check if this plugin is valid
 */
bool Plugin::isValid()
{
	XOJ_CHECK_TYPE(Plugin);

	return valid;
}

#endif

