#include "Plugin.h"

#include <config.h>
#include <i18n.h>

#ifdef ENABLE_PLUGINS

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "luapi_application.h"

#define LOAD_FROM_INI(target, group, key) \
	{ \
		char* value = g_key_file_get_string(config, group, key, nullptr); \
		if (value != nullptr) \
		{ \
			target = value; \
			g_free(value); \
		} \
	}

/*
 ** these libs are loaded by lua.c and are readily available to any Lua
 ** program
 */
static const luaL_Reg loadedlibs[] = {
	{ "app", luaopen_app },
	{ nullptr, nullptr }
};

Plugin::Plugin(Control* control, string name, string path)
 : control(control),
   name(name),
   path(path)
{
	loadIni();
}

Plugin::~Plugin()
{
	if (lua)
	{
		// Clean up, free the Lua state var
		lua_close(lua);
		lua = nullptr;
	}

	for (MenuEntry* m : menuEntries)
	{
		delete m;
	}
	menuEntries.clear();
}

/**
 * Get Plugin from lua engine
 */
Plugin* Plugin::getPluginFromLua(lua_State* lua)
{
	lua_getfield(lua, LUA_REGISTRYINDEX, "Xournalpp_Plugin");

	if (lua_islightuserdata(lua, -1))
	{
		Plugin* data = (Plugin*)lua_touserdata(lua, -1);
		lua_pop(lua, 1);
	return data;
	}

	return nullptr;
}

/**
 * Register toolbar item and all other UI stuff
 */
void Plugin::registerToolbar()
{
	if (!this->valid || !this->enabled)
	{
		return;
	}

	inInitUi = true;

	lua_getglobal(lua, "initUi");
	if (lua_isfunction (lua, -1) == 1)
	{
		if (callFunction("initUi"))
		{
			g_message("Plugin «%s» UI initialized", name.c_str());
		}
		else
		{
			g_warning("Plugin «%s» init failed!", name.c_str());
		}
	}
	else
	{
		g_message("Plugin «%s» has no UI init", name.c_str());
	}

	inInitUi = false;
}


/**
 * Register all menu entries to the menu
 */
void Plugin::registerMenu(GtkWindow* mainWindow, GtkWidget* menu)
{
	if (menuEntries.empty() || !this->enabled)
	{
		// No entries - nothing to do
		return;
	}

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

	GtkAccelGroup* accelGroup = gtk_accel_group_new();

	for (MenuEntry* m : menuEntries)
	{
		GtkWidget* mi = gtk_menu_item_new_with_label(m->menu.c_str());
		m->widget = mi;
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

		if (m->accelerator != "")
		{
			guint acceleratorKey = 0;
			GdkModifierType mods = (GdkModifierType)0;
			gtk_accelerator_parse(m->accelerator.c_str(), &acceleratorKey, &mods);

			gtk_widget_add_accelerator(mi, "activate", accelGroup, acceleratorKey, mods, GTK_ACCEL_VISIBLE);
		}

		g_signal_connect(mi, "activate", G_CALLBACK(
			+[](GtkWidget* bt, MenuEntry* me)
			{
	me->plugin->executeMenuEntry(me);
			}), m);
	}

	gtk_window_add_accel_group(GTK_WINDOW(mainWindow), accelGroup);
}

/**
 * Execute menu entry
 */
void Plugin::executeMenuEntry(MenuEntry* entry)
{
	callFunction(entry->callback);
}

/**
 * @return the Plugin name
 */
string Plugin::getName()
{
	return name;
}

/**
 * @return Description of the plugin
 */
string Plugin::getDescription()
{
	return description;
}

/**
 * Author of the plugin
 */
string Plugin::getAuthor()
{
	return author;
}

/**
 * Plugin version
 */
string Plugin::getVersion()
{
	return version;
}

/**
 * The plugin is enabled
 */
bool Plugin::isEnabled()
{
	return enabled;
}

/**
 * The plugin is enabled
 */
void Plugin::setEnabled(bool enabled)
{
	this->enabled = enabled;
}

/**
 * The plugin is default enabled
 */
bool Plugin::isDefaultEnabled()
{
	return defaultEnabled;
}

/**
 * @return Flag to check if init ui is currently running
 */
bool Plugin::isInInitUi()
{
	return inInitUi;
}

/**
 * Register a menu item
 *
 * @return Internal ID, can e.g. be used to disable the menu
 */
int Plugin::registerMenu(string menu, string callback, string accelerator)
{
	MenuEntry* m = new MenuEntry(this);
	m->menu = menu;
	m->callback = callback;
	m->accelerator = accelerator;
	menuEntries.push_back(m);

	return menuEntries.size() - 1;
}

/**
 * @return The main controller
 */
Control* Plugin::getControl()
{
	return control;
}

/**
 * Load ini file
 */
void Plugin::loadIni()
{
	GKeyFile* config = g_key_file_new();
	g_key_file_set_list_separator(config, ',');

	string filename = path + "/plugin.ini";
	if (!g_key_file_load_from_file(config, filename.c_str(), G_KEY_FILE_NONE, nullptr))
	{
		g_key_file_free(config);
		return;
	}

	LOAD_FROM_INI(author, "about", "author");
	LOAD_FROM_INI(version, "about", "version");
	LOAD_FROM_INI(description, "about", "description");

	if (version == "<xournalpp>")
	{
		version = PROJECT_VERSION;
	}

	LOAD_FROM_INI(mainfile, "plugin", "mainfile");

	string defaultEnabledStr;
	LOAD_FROM_INI(defaultEnabledStr, "default", "enabled");

	defaultEnabled = defaultEnabledStr == "true";
	enabled = defaultEnabled;

	g_key_file_free(config);

	this->valid = true;
}

/**
 * Load custom Lua Libraries
 */
void Plugin::registerXournalppLibs(lua_State* lua)
{
	for (const luaL_Reg* lib = loadedlibs; lib->func; lib++)
	{
		luaL_requiref(lua, lib->name, lib->func, 1);

		// remove lib
		lua_pop(lua, 1);
	}
}

/**
 * Add the plugin folder to the lua path
 */
void Plugin::addPluginToLuaPath()
{
	lua_getglobal(lua, "package");

	// get field "path" from table at top of stack (-1)
	lua_getfield(lua, -1, "path");

	// For now: limit the include path to the current plugin folder, for security and compatibility reasons
	// grab path string from top of stack
	// string curPath = lua_tostring(lua, -1);
	// curPath.append(";");
	string curPath;
	curPath.append(path);
	curPath.append("/?.lua");

	// get rid of the string on the stack we just pushed on line 5
	lua_pop(lua, 1);

	// push the new one
	lua_pushstring(lua, curPath.c_str());

	// set the field "path" in table at -2 with value at top of stack
	lua_setfield(lua, -2, "path");

	// get rid of package table from top of stack
	lua_pop(lua, 1);
}

/**
 * Load the plugin script
 */
void Plugin::loadScript()
{
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

	if (!this->enabled)
	{
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
	lua_pushlightuserdata(lua, this);
	lua_setfield(lua, LUA_REGISTRYINDEX, "Xournalpp_Plugin");

	registerXournalppLibs(lua);

	addPluginToLuaPath();

	// Run the loaded Lua script
	if (lua_pcall(lua, 0, 0, 0) != LUA_OK)
	{
		const char* errMsg = lua_tostring(lua, -1);
		map<int, string> button;
		button.insert(std::pair<int, string>(0, _("OK")));
		XojMsgBox::showPluginMessage(name, errMsg, button, true);

		g_warning("Could not run plugin Lua file: «%s», error: «%s»", luafile.c_str(), errMsg);
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
		const char* errMsg = lua_tostring(lua, -1);
		map<int, string> button;
		button.insert(std::pair<int, string>(0, _("OK")));
		XojMsgBox::showPluginMessage(name, errMsg, button, true);

		g_warning("Error in Plugin: «%s», error: «%s»", name.c_str(), errMsg);
		return false;
	}

	return true;
}

/**
 * Check if this plugin is valid
 */
bool Plugin::isValid()
{
	return valid;
}

#endif

