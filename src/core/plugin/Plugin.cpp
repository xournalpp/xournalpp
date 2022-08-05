#include "Plugin.h"

#include <algorithm>  // for max
#include <array>      // for array
#include <map>        // for map

#include <gdk/gdk.h>      // for GdkModifierType
#include <glib-object.h>  // for G_CALLBACK, g_signal_connect
#include <glib.h>         // for g_key_file_free, g_warning, g_free

#include "util/PathUtil.h"   // for toGFilename
#include "util/XojMsgBox.h"  // for XojMsgBox
#ifdef ENABLE_PLUGINS

#include <utility>  // for move, pair

#include "util/i18n.h"  // for _

#include "config.h"  // for PROJECT_VERSION

extern "C" {
#include <lauxlib.h>  // for luaL_Reg, luaL_newstate, luaL_requiref
#include <lua.h>      // for lua_getglobal, lua_getfield, lua_setf...
#include <lualib.h>   // for luaL_openlibs
}

#include "luapi_application.h"  // for luaopen_app

/*
 ** these libs are loaded by lua.c and are readily available to any Lua
 ** program
 */
constexpr std::array loadedlibs{luaL_Reg{"app", luaopen_app}};

Plugin::Plugin(Control* control, std::string name, fs::path path):
        control(control), name(std::move(name)), path(std::move(path)) {
    loadIni();
}

auto Plugin::getPluginFromLua(lua_State* lua) -> Plugin* {
    lua_getfield(lua, LUA_REGISTRYINDEX, "Xournalpp_Plugin");

    if (lua_islightuserdata(lua, -1)) {
        auto* data = static_cast<Plugin*>(lua_touserdata(lua, -1));
        lua_pop(lua, 1);
        return data;
    }

    return nullptr;
}

void Plugin::registerToolbar() {
    if (!this->valid || !this->enabled) {
        return;
    }

    inInitUi = true;

    lua_getglobal(lua.get(), "initUi");
    if (lua_isfunction(lua.get(), -1) == 1) {
        if (callFunction("initUi")) {
            g_message("Plugin \"%s\" UI initialized", name.c_str());
        } else {
            g_warning("Plugin \"%s\" init failed!", name.c_str());
        }
    } else {
        g_message("Plugin \"%s\" has no UI init", name.c_str());
    }

    inInitUi = false;
}

void Plugin::registerMenu(GtkWindow* mainWindow, GtkWidget* menu) {
    if (menuEntries.empty() || !this->enabled) {
        // No entries - nothing to do
        return;
    }

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

    GtkAccelGroup* accelGroup = gtk_accel_group_new();

    for (auto&& m: menuEntries) {
        GtkWidget* mi = gtk_menu_item_new_with_label(m.menu.c_str());
        m.widget = mi;
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

        if (!m.accelerator.empty()) {
            auto acceleratorKey = guint(0);
            auto mods = GdkModifierType(0);
            gtk_accelerator_parse(m.accelerator.c_str(), &acceleratorKey, &mods);
            gtk_widget_add_accelerator(mi, "activate", accelGroup, acceleratorKey, mods, GTK_ACCEL_VISIBLE);
        }

        // This might fail, when the vector reallocates, but then the order of initialisation is violated
        g_signal_connect(mi, "activate",
                         G_CALLBACK(+[](GtkWidget* bt, MenuEntry* me) { me->plugin->executeMenuEntry(me); }), &m);
    }

    gtk_window_add_accel_group(GTK_WINDOW(mainWindow), accelGroup);
}

void Plugin::executeMenuEntry(MenuEntry* entry) { callFunction(entry->callback, entry->mode); }

auto Plugin::registerMenu(std::string menu, std::string callback, long mode, std::string accelerator) -> size_t {
    menuEntries.emplace_back(this, std::move(menu), std::move(callback), mode, std::move(accelerator));
    return menuEntries.size() - 1;
}

auto Plugin::getControl() const -> Control* { return control; }

void Plugin::loadIni() {
    GKeyFile* config = g_key_file_new();
    g_key_file_set_list_separator(config, ',');

    auto filename = path / "plugin.ini";
    if (!g_key_file_load_from_file(config, Util::toGFilename(filename).c_str(), G_KEY_FILE_NONE, nullptr)) {
        g_key_file_free(config);
        return;
    }

    auto LOAD_FROM_INI = [config](auto& target, char const* group, char const* key) {
        char* value = g_key_file_get_string(config, group, key, nullptr);
        if (value != nullptr) {
            target = value;
            g_free(value);
        }
    };

    LOAD_FROM_INI(author, "about", "author");
    LOAD_FROM_INI(version, "about", "version");
    LOAD_FROM_INI(description, "about", "description");

    if (version == "<xournalpp>") {
        version = PROJECT_VERSION;
    }

    LOAD_FROM_INI(mainfile, "plugin", "mainfile");

    std::string defaultEnabledStr;
    LOAD_FROM_INI(defaultEnabledStr, "default", "enabled");

    defaultEnabled = defaultEnabledStr == "true";
    enabled = defaultEnabled;

    g_key_file_free(config);

    this->valid = true;
}

void Plugin::registerXournalppLibs(lua_State* luaPtr) {
    for (auto const& lib: loadedlibs) {
        luaL_requiref(luaPtr, lib.name, lib.func, 1);
        // remove lib
        lua_pop(luaPtr, 1);
    }
}

void Plugin::addPluginToLuaPath() {
    lua_getglobal(lua.get(), "package");

    // get field "path" from table at top of stack (-1)
    lua_getfield(lua.get(), -1, "path");

    // grab path string from top of stack
    std::string luaPath = lua_tostring(lua.get(), -1);

    // prepend the path of the current plugin
    auto curPath = this->path / "?.lua";
    std::string combinedPath = curPath.string() + ";" + luaPath;

    // get rid of the std::string on the stack we just pushed
    lua_pop(lua.get(), 1);

    // push the new one
    lua_pushstring(lua.get(), combinedPath.c_str());

    // set the field "path" in table at -2 with value at top of stack
    lua_setfield(lua.get(), -2, "path");

    // get rid of package table from top of stack
    lua_pop(lua.get(), 1);
}

void Plugin::loadScript() {
    if (mainfile.empty()) {
        this->valid = false;
        return;
    }

    if (mainfile.find("..") != std::string::npos) {
        g_warning("Plugin \"%s\" contains unsupported path \"%s\"", name.c_str(), mainfile.c_str());
        this->valid = false;
        return;
    }

    if (!this->enabled) {
        return;
    }


    // Create Lua state variable
    lua.reset(luaL_newstate());

    // Load Lua libraries
    luaL_openlibs(lua.get());

    // Load but don't run the Lua script
    auto luafile = path / mainfile;
    if (luaL_loadfile(lua.get(), luafile.string().c_str())) {
        // Error out if file can't be read
        g_warning("Could not run plugin Lua file: \"%s\"", luafile.string().c_str());
        this->valid = false;
        return;
    }

    // Register Plugin object to Lua instance
    lua_pushlightuserdata(lua.get(), this);
    lua_setfield(lua.get(), LUA_REGISTRYINDEX, "Xournalpp_Plugin");

    registerXournalppLibs(lua.get());

    addPluginToLuaPath();

    // Run the loaded Lua script
    if (lua_pcall(lua.get(), 0, 0, 0) != LUA_OK) {
        const char* errMsg = lua_tostring(lua.get(), -1);
        std::map<int, std::string> button;
        button.insert(std::pair<int, std::string>(0, _("OK")));
        XojMsgBox::showPluginMessage(name, errMsg, button, true);

        g_warning("Could not run plugin Lua file: \"%s\", error: \"%s\"", luafile.string().c_str(), errMsg);
        this->valid = false;
        return;
    }
}

auto Plugin::callFunction(const std::string& fnc, long mode) -> bool {
    lua_getglobal(lua.get(), fnc.c_str());

    int numArgs = 0;

    if (mode != LONG_MAX) {
        lua_pushinteger(lua.get(), mode);
        numArgs = 1;
    }

    // Run the function
    if (lua_pcall(lua.get(), numArgs, 0, 0)) {
        const char* errMsg = lua_tostring(lua.get(), -1);
        std::map<int, std::string> button;
        button.insert(std::pair<int, std::string>(0, _("OK")));
        XojMsgBox::showPluginMessage(name, errMsg, button, true);

        g_warning("Error in Plugin: \"%s\", error: \"%s\"", name.c_str(), errMsg);
        return false;
    }

    return true;
}

auto Plugin::getName() const -> std::string const& { return name; }
auto Plugin::getDescription() const -> std::string const& { return description; }
auto Plugin::getAuthor() const -> std::string const& { return author; }
auto Plugin::getVersion() const -> std::string const& { return version; }
auto Plugin::getPath() const -> fs::path const& { return path; }

auto Plugin::isEnabled() const -> bool { return enabled; }
void Plugin::setEnabled(bool lEnabled) { this->enabled = lEnabled; }
auto Plugin::isDefaultEnabled() const -> bool { return defaultEnabled; }

auto Plugin::isInInitUi() const -> bool { return inInitUi; }
auto Plugin::isValid() const -> bool { return valid; }

#endif
