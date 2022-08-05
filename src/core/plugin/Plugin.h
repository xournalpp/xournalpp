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

#include <cstddef>  // for size_t
#include <memory>   // for unique_ptr

#include "config-features.h"  // for ENABLE_PLUGINS

#ifdef ENABLE_PLUGINS

#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector

#include <gtk/gtk.h>  // for GtkWidget, GtkWindow

#include "filesystem.h"  // for path

extern "C" {
#include <lua.h>  // for lua_State, lua_close
}

class Plugin;
class Control;

struct MenuEntry final {
    MenuEntry() = default;
    MenuEntry(Plugin* plugin, std::string menu, std::string callback, long mode, std::string accelerator):
            plugin(plugin),
            menu(std::move(menu)),
            callback(std::move(callback)),
            mode(mode),
            accelerator(std::move(accelerator)) {}

    GtkWidget* widget = nullptr;  ///< Menu item
    Plugin* plugin = nullptr;     ///< The Plugin
    std::string menu{};           ///< Menu display name
    std::string callback{};       ///< Callback function name
    long mode{LONG_MAX};          ///< mode in which the callback function is run
    std::string accelerator{};
    ///< Accelerator key, see
    ///< https://developer.gnome.org/gtk3/stable/gtk3-Keyboard-Accelerators.html#gtk-accelerator-parse
};

struct LuaDeleter {
    void operator()(lua_State* ptr) const { lua_close(ptr); }
};

class Plugin final {
public:
    Plugin(Control* control, std::string name, fs::path path);

public:
    /// Load the plugin script
    void loadScript();

    /// Check if this plugin is valid
    auto isValid() const -> bool;

    /// Register toolbar item and all other UI stuff
    void registerToolbar();

    /// Register all menu entries to the menu
    void registerMenu(GtkWindow* mainWindow, GtkWidget* menu);

    /// Execute menu entry
    void executeMenuEntry(MenuEntry* entry);

    /// @return the Plugin name
    auto getName() const -> std::string const&;

    /// @return Description of the plugin
    auto getDescription() const -> std::string const&;

    /// Author of the plugin
    auto getAuthor() const -> std::string const&;

    /// Plugin version
    auto getVersion() const -> std::string const&;

    /// @return the path to the plugin folder. Each plugin should have a unique path.
    auto getPath() const -> fs::path const&;

    /// The plugin is enabled
    auto isEnabled() const -> bool;

    /// The plugin is enabled
    void setEnabled(bool lEnabled);

    /// The plugin is default enabled
    auto isDefaultEnabled() const -> bool;

    ///@return Flag to check if init ui is currently running
    auto isInInitUi() const -> bool;

    /// Register a menu item
    /// @return Internal ID, can e.g. be used to disable the menu
    auto registerMenu(std::string menu, std::string callback, long mode, std::string accelerator) -> size_t;

    ///@return The main controller
    auto getControl() const -> Control*;

private:
    /// Load ini file
    void loadIni();

    /// Execute lua function
    auto callFunction(const std::string& fnc, long mode = LONG_MAX) -> bool;

    /// Load custom Lua Libraries
    static void registerXournalppLibs(lua_State* luaPtr);

    /// Add the plugin folder to the lua path
    void addPluginToLuaPath();

public:
    /// Get Plugin from lua engine
    static auto getPluginFromLua(lua_State* lua) -> Plugin*;

private:
    Control* control;                              ///< The main controller
    std::unique_ptr<lua_State, LuaDeleter> lua{};  ///< Lua engine
    std::vector<MenuEntry> menuEntries;            ///< All registered menu entries

    std::string name;             ///< Plugin name
    std::string description;      ///< Description of the plugin
    std::string author;           ///< Author of the plugin
    std::string version;          ///< Plugin version
    std::string mainfile;         ///< Main plugin script
    fs::path path;                ///< Plugin root path
    bool enabled = false;         ///< The plugin is enabled
    bool defaultEnabled = false;  ///< The plugin is default enabled
    bool inInitUi = false;        ///< Flag to check if init ui is currently running
    bool valid = false;           ///< Flag if the plugin is valid / correct loaded
};

#else
struct Plugin final {};  ///< empty struct, since Plugins are not available
#endif
