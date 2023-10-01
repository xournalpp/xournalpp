/*
 * Xournal++
 *
 * Lua API, application library
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <climits>
#include <cmath>  // for rounding
#include <cstring>
#include <limits>  // for numeric_limits
#include <map>

#include <gtk/gtk.h>
#include <stdint.h>

#include "control/Control.h"
#include "control/ExportHelper.h"
#include "control/PageBackgroundChangeController.h"
#include "control/ScrollHandler.h"
#include "control/Tool.h"
#include "control/layer/LayerController.h"
#include "control/pagetype/PageTypeHandler.h"
#include "control/settings/Settings.h"
#include "control/tools/EditSelection.h"
#include "control/tools/ImageHandler.h"
#include "gui/Layout.h"
#include "gui/MainWindow.h"
#include "gui/XournalView.h"
#include "gui/sidebar/Sidebar.h"
#include "gui/sidebar/previews/base/SidebarToolbar.h"
#include "gui/widgets/XournalWidget.h"
#include "model/Document.h"
#include "model/Font.h"
#include "model/SplineSegment.h"
#include "model/Stroke.h"
#include "model/StrokeStyle.h"
#include "model/Text.h"
#include "model/XojPage.h"
#include "model/path/PiecewiseLinearPath.h"
#include "model/path/Spline.h"
#include "plugin/Plugin.h"
#include "undo/InsertUndoAction.h"
#include "util/StringUtils.h"
#include "util/XojMsgBox.h"
#include "util/i18n.h"  // for _
#include "util/safe_casts.h"

extern "C" {
#include <lauxlib.h>  // for luaL_Reg, luaL_newstate, luaL_requiref
#include <lua.h>      // for lua_getglobal, lua_getfield, lua_setf...
#include <lualib.h>   // for luaL_openlibs
}

/*
 * Code conventions:
 *     Error handling:
 *         luapi functions should call `return luaL_error(L, fmt, ...)` if
 *         something *unexpected happens* (e.g. wrong arguments). This throws a
 *         real lua error.
 *         They may also `return nil, errorMessage`. This behavior is reserved for
 *         things that are *expected to happen* (e.g. ressource is not
 *         available).
 */

/**
 * Renames file 'from' to file 'to' in the file system.
 * Overwrites 'to' if it already exists.
 *
 * Example:
 *   assert(app.glib_rename("path/to/foo", "other/bar"))
 *
 * Preferred to os.rename() because it works across
 * partitions. Uses glib's rename function.
 *
 * Returns 1 on success, and (nil, message) on failure.
 */
static int applib_glib_rename(lua_State* L) {
    GError* err = nullptr;
    xoj::util::GObjectSPtr<GFile> to(g_file_new_for_path(lua_tostring(L, -1)), xoj::util::adopt);
    xoj::util::GObjectSPtr<GFile> from(g_file_new_for_path(lua_tostring(L, -2)), xoj::util::adopt);

    g_file_move(from.get(), to.get(), G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, &err);
    if (err) {
        // return nil, error message
        lua_pushnil(L);
        lua_pushfstring(L, "%s (error code: %d)", err->message, err->code);
        g_error_free(err);
        return 2;
    } else {
        // return 1
        lua_pushinteger(L, 1);
        return 1;
    }
}


/**
 * Create a 'Save As' native dialog and return as a string
 * the filepath of the location the user chose to save.
 *
 * Examples:
 *   local filename = app.saveAs() -- defaults to suggestion "Untitled"
 *   local filename = app.saveAs("foo") -- suggests "foo" as filename
 */
static int applib_saveAs(lua_State* L) {
    gint res;
    int args_returned = 0;  // change to 1 if user chooses file

    const char* filename = luaL_checkstring(L, -1);

    // Create a 'Save As' native dialog
    xoj::util::GObjectSPtr<GtkFileChooserNative> native(
            gtk_file_chooser_native_new(_("Save file"), nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, nullptr, nullptr),
            xoj::util::adopt);

    // If user tries to overwrite a file, ask if it's OK
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(native.get()), TRUE);
    // Offer a suggestion for the filename if filename absent
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native.get()),
                                      filename ? filename : (std::string{_("Untitled")}).c_str());

    // Wait until user responds to dialog
    res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native.get()));

    // Return the filename chosen to lua
    if (res == GTK_RESPONSE_ACCEPT) {
        char* filename = static_cast<char*>(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native.get())));

        lua_pushlstring(L, filename, strlen(filename));
        g_free(static_cast<gchar*>(filename));
        args_returned = 1;
    }

    return args_returned;
}

/**
 * Create a 'Open File' native dialog and return as a string
 * the filepath the user chose to open.
 *
 * Examples:
 *   path = app.getFilePath()
 *   path = app.getFilePath({'*.bmp', '*.png'})
 */
static int applib_getFilePath(lua_State* L) {
    xoj::util::GObjectSPtr<GtkFileChooserNative> native(
            gtk_file_chooser_native_new(_("Open file"), nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, nullptr, nullptr),
            xoj::util::adopt);
    gint res;
    int args_returned = 0;  // change to 1 if user chooses file
    char* filename;

    // Get vector of supported formats from Lua stack
    std::vector<std::string> formats;
    // stack now contains: -1 => table
    lua_pushnil(L);
    // stack now contains: -1 => nil; -2 => table
    while (lua_next(L, -2)) {
        // stack now contains: -1 => value; -2 => key; -3 => table
        const char* value = lua_tostring(L, -1);
        formats.push_back(value);
        lua_pop(L, 1);
        // stack now contains: -1 => key; -2 => table
    }
    // stack now contains: -1 => table
    lua_pop(L, 1);  // Stack is now the same as it was on entry to this function
    if (formats.size() > 0) {
        GtkFileFilter* filterSupported = gtk_file_filter_new();
        gtk_file_filter_set_name(filterSupported, _("Supported files"));
        for (std::string format: formats) gtk_file_filter_add_pattern(filterSupported, format.c_str());
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native.get()), filterSupported);
    }

    // Wait until user responds to dialog
    res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native.get()));
    // Return the filename chosen to lua
    if (res == GTK_RESPONSE_ACCEPT) {
        filename = static_cast<char*>(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native.get())));
        lua_pushlstring(L, filename, strlen(filename));
        g_free(static_cast<gchar*>(filename));
        args_returned = 1;
    }
    // Destroy the dialog and free memory
    return args_returned;
}

/**
 * THIS FUNCTION IS DEPRECATED AND WILL BE REMOVED SOON. Use applib_openDialog() instead.
 *
 * Example: local result = app.msgbox("Test123", {[1] = "Yes", [2] = "No"})
 * Pops up a message box with two buttons "Yes" and "No" and returns 1 for yes, 2 for no
 */
static int applib_msgbox(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);

    // discard any extra arguments passed in
    lua_settop(L, 2);
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_pushnil(L);  // initial key for table traversal with `next`

    std::vector<XojMsgBox::Button> buttons;

    while (lua_next(L, 2) != 0) {
        int index = static_cast<int>(lua_tointeger(L, -2));
        const char* buttonText = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        buttons.emplace_back(buttonText, index);
    }

    Plugin* plugin = Plugin::getPluginFromLua(L);

    int result = XojMsgBox::askPluginQuestion(plugin->getName(), msg, buttons);
    lua_pushinteger(L, result);
    return 1;
}

/**
 * Example 1: app.openDialog("Test123", {[1] = "Yes", [2] = "No"}, "cb", false)
 *   or       app.openDialog("Test123", {"Yes", "No"}, "cb")
 * Pops up a message box with two buttons "Yes" and "No" and executed function "cb" whose single parameter is the number
 * corresponding to the button the user clicked.
 *
 * If the optional boolean parameter is true, the dialog is treated as an error message
 * Example 2: app.openDialog("Invalid parameter", {"Ok"}, "", true)
 *
 * Warning: the callback function is never called if the dialog is closed without pressing one of the custom buttons.
 */
static int applib_openDialog(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);

    // discard any extra arguments passed in
    lua_settop(L, 4);
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_pushnil(L);  // initial key for table traversal with `next`

    std::vector<XojMsgBox::Button> buttons;

    while (lua_next(L, 2) != 0) {
        int index = static_cast<int>(lua_tointeger(L, -2));
        const char* buttonText = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        buttons.emplace_back(buttonText, index);
    }

    std::string cb = luaL_optstring(L, 3, "");
    bool error = lua_toboolean(L, 4);

    Plugin* plugin = Plugin::getPluginFromLua(L);

    const std::string& pluginName = plugin->getName();
    auto header = (error ? std::string("<b>Error in </b>") : "") + std::string("Xournal++ Plugin «") + pluginName + "»";

    XojMsgBox::askQuestionWithMarkup(nullptr, header, msg, buttons, [cb, plugin](int response) {
        if (cb != "" && response >= 1) {
            plugin->callFunction(cb, static_cast<long>(response));
        }
    });

    return 0;
}


/**
 * Allow to register menupoints and toolbar buttons. This needs to be called from initUi
 *
 * Example 1: app.registerUi({["menu"] = "HelloWorld", callback="printMessage", mode=1, accelerator="<Control>a"})
 * registers a menupoint with name "HelloWorld" executing a function named "printMessage", in mode 1,
 * which can be triggered via the "<Control>a" keyboard accelerator
 *
 * Example 2: app.registerUi({callback ="blueDashedPen", toolbarId="CUSTOM_PEN_1", iconName="bluePenIcon"})
 * registers a toolbar icon named "bluePenIcon" executing a function named "blueDashedPen", which can be added
 * to a toolbar via toolbar customization or by editing the toolbar.ini file using the name "Plugin::CUSTOM_PEN_1"
 * Note that in toolbar.ini the string "Plugin::" must always be prepended to the toolbarId specified in the plugin
 *
 * The mode and accelerator are optional. When specifying the mode, the callback function should have one parameter
   that receives the mode. This is useful for callback functions that are shared among multiple menu entries.
 */
static int applib_registerUi(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    if (!plugin->isInInitUi()) {
        return luaL_error(L, "registerUi needs to be called within initUi()");
    }

    // discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    // Now to get the data out of the table
    // 'unpack' the table by putting the values onto
    // the stack first. Then convert those stack values
    // into an appropriate C type.
    lua_getfield(L, 1, "accelerator");
    lua_getfield(L, 1, "menu");
    lua_getfield(L, 1, "callback");
    lua_getfield(L, 1, "mode");
    lua_getfield(L, 1, "toolbarId");
    lua_getfield(L, 1, "iconName");
    // In Example 1 stack now has following:
    //    1 = {"menu"="MenuName", callback="functionName", mode=1, accelerator="<Control>a"}
    //   -6 = "<Control>a"
    //   -5 = "MenuName"
    //   -4 = "functionName"
    //   -3 = mode
    //   -2 = nil
    //   -1 = nil

    const char* accelerator = luaL_optstring(L, -6, "");
    const char* menu = luaL_optstring(L, -5, "");
    const char* callback = luaL_optstring(L, -4, nullptr);
    const long mode = luaL_optinteger(L, -3, std::numeric_limits<long>::max());
    const char* toolbarId = luaL_optstring(L, -2, "");
    const char* iconName = luaL_optstring(L, -1, "");
    if (callback == nullptr) {
        return luaL_error(L, "Missing callback function!");
    }

    int menuId = plugin->registerMenu(menu, callback, mode, accelerator);
    plugin->registerToolButton(menu, toolbarId, iconName, callback, mode);

    // Make sure to remove all vars which are put to the stack before!
    lua_pop(L, 6);

    // Add return value to the Stack
    lua_createtable(L, 0, 2);

    lua_pushstring(L, "menuId");
    lua_pushinteger(L, menuId);
    lua_settable(L, -3); /* 3rd element from the stack top */

    return 1;
}

/**
 * Execute an UI action (usually internally called from Toolbar / Menu)
 * The argument consists of a Lua table with 3 keys: "action", "group" and "enabled"
 * The key "group" is currently only used for debugging purpose and can safely be omitted.
 * The key "enabled" is true by default.
 *
 * Example 1: app.uiAction({["action"] = "ACTION_PASTE"})
 * pastes the clipboard content into the document
 *
 * Example 2: app.uiAction({["action"] = "ACTION_TOOL_DRAW_ELLIPSE", ["enabled"] = false})
 * turns off the Ellipse drawing type
 */
static int applib_uiAction(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);

    // discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "group");
    lua_getfield(L, 1, "enabled");
    lua_getfield(L, 1, "action");
    // stack now has following:
    //    1 = {["action"] = "ACTION_GRID_SNAPPING", ["group"] = "GROUP_GRID_SNAPPING", ["enabled"] = true}
    //   -3 = GROUP_GRID_SNAPPING
    //   -2 = true
    //   -1 = "ACTION_GRID_SNAPPING"

    bool enabled = true;

    ActionGroup group = GROUP_NOGROUP;
    const char* groupStr = luaL_optstring(L, -3, nullptr);
    if (groupStr != nullptr) {
        group = ActionGroup_fromString(groupStr);
    }

    if (lua_isboolean(L, -2)) {
        enabled = lua_toboolean(L, -2);
    }

    const char* actionStr = luaL_optstring(L, -1, nullptr);
    if (actionStr == nullptr) {
        return luaL_error(L, "Missing action!");
    }

    ActionType action = ActionType_fromString(actionStr);
    GtkToolButton* toolbutton = nullptr;

    Control* ctrl = plugin->getControl();
    ctrl->actionPerformed(action, group, toolbutton, enabled);

    return 0;
}

/**
 * Select UI action (notifies action listeners)
 * Unless you are sure what you do, use app.uiAction instead!
 * The problem is that only notifying action listeners does not store these changes in the settings, which may create
 * confusion
 *
 * Example: app.uiActionSelected("GROUP_GRID_SNAPPING", "ACTION_GRID_SNAPPING")
 * notifies the action listeners that grid snapping is turned on; it is not recorded in the settings,
 * so better use app.uiAction({["action"] = "ACTION_GRID_SNAPPING") instead
 */
static int applib_uiActionSelected(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);

    ActionGroup group = ActionGroup_fromString(luaL_checkstring(L, 1));
    ActionType action = ActionType_fromString(luaL_checkstring(L, 2));

    Control* ctrl = plugin->getControl();
    ctrl->fireActionSelected(group, action);

    return 0;
}

/**
 * Execute action from sidebar menu
 *
 * Example: app.sidebarAction("MOVE_DOWN")
 * moves down the current page
 */
static int applib_sidebarAction(lua_State* L) {
    // Connect the context menu actions
    const std::map<std::string, SidebarActions> actionMap = {
            {"COPY", SIDEBAR_ACTION_COPY},
            {"DELETE", SIDEBAR_ACTION_DELETE},
            {"MOVE_UP", SIDEBAR_ACTION_MOVE_UP},
            {"MOVE_DOWN", SIDEBAR_ACTION_MOVE_DOWN},
            {"NEW_BEFORE", SIDEBAR_ACTION_NEW_BEFORE},
            {"NEW_AFTER", SIDEBAR_ACTION_NEW_AFTER},
    };
    const char* actionStr = luaL_checkstring(L, 1);
    if (actionStr == nullptr) {
        return luaL_error(L, "Missing action!");
    }
    auto pos = actionMap.find(actionStr);
    if (pos == actionMap.end()) {
        return luaL_error(L, "Unknown action: %s", actionStr);
    }
    Plugin* plugin = Plugin::getPluginFromLua(L);
    SidebarToolbar* toolbar = plugin->getControl()->getSidebar()->getToolbar();
    toolbar->runAction(pos->second);

    return 0;
}

/*
 * Get the index of the currently active sidebar-page.
 *
 * Example: app.getSidebarPageNo() -- returns e.g. 1
 */

static int applib_getSidebarPageNo(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Sidebar* sidebar = plugin->getControl()->getSidebar();
    lua_pushinteger(L, sidebar->getSelectedPage() + 1);
    return 1;
}

/*
 * Set the currently active sidebar-page by its index.
 *
 * Look at src/core/gui/sidebar/Sidebar.cpp to find out which index corresponds to which page (e.g. currently 1 is the
 * page with the TOC/index if available). Note that indexing the sidebar-pages starts at 1 (as usual in lua).
 *
 * Example: app.setSidebarPageNo(3) -- sets the sidebar-page to preview Layer
 */
static int applib_setSidebarPageNo(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Sidebar* sidebar = plugin->getControl()->getSidebar();

    // Discard any extra arguments passed in
    lua_settop(L, 1);
    if (!lua_isinteger(L, 1)) {
        return luaL_error(L, "Missing pageNo for setSidebarPageNo!");
    }

    int page = lua_tointeger(L, 1);
    if (page <= 0) {
        lua_pushnil(L);
        lua_pushfstring(L, "Invalid pageNo (%d) provided!", page);
        return 2;
    }
    if (page > sidebar->getNumberOfPages()) {
        lua_pushnil(L);
        lua_pushfstring(L, "Invalid pageNo (%d >= %d) provided!", page, sidebar->getNumberOfPages());
        return 2;
    }

    sidebar->setSelectedPage(page - 1);
    return 0;
}

/**
 * Execute action from layer controller
 *
 * Example: app.layerAction("ACTION_DELETE_LAYER")
 * deletes the current layer
 */
static int applib_layerAction(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);

    ActionType action = ActionType_fromString(luaL_checkstring(L, 1));

    Control* ctrl = plugin->getControl();
    ctrl->getLayerController()->actionPerformed(action);

    return 0;
}

/**
 * Helper function to handle a allowUndoRedoAction string parameter. allowUndoRedoAction can take the following values:
 * - "grouped": the elements get a single undo-redo-action
 * - "individual" each of the elements get an own undo-redo-action
 * - "none": no undo-redo-action will be inserted
 * if an invalid value is being passed as allowUndoRedoAction this function errors
 */
static int handleUndoRedoActionHelper(lua_State* L, Control* control, const char* allowUndoRedoAction,
                                      const std::vector<Element*>& elements) {
    if (strcmp("grouped", allowUndoRedoAction) == 0) {
        PageRef const& page = control->getCurrentPage();
        Layer* layer = page->getSelectedLayer();
        UndoRedoHandler* undo = control->getUndoRedoHandler();
        undo->addUndoAction(std::make_unique<InsertsUndoAction>(page, layer, elements));
    } else if (strcmp("individual", allowUndoRedoAction) == 0) {
        PageRef const& page = control->getCurrentPage();
        Layer* layer = page->getSelectedLayer();
        UndoRedoHandler* undo = control->getUndoRedoHandler();
        for (Element* element: elements) undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, element));
    } else if (strcmp("none", allowUndoRedoAction) == 0)
        g_warning("Not allowing undo/redo action.");
    else {
        return luaL_error(L, "Unrecognized undo/redo option: %s", allowUndoRedoAction);
    }
    return 0;
}


/**
 * Helper function for addStroke API. Parses pen settings from API call, taking
 * in a Stroke and a chosen Layer, sets the pen settings, and applies the stroke.
 */
static void addStrokeHelper(lua_State* L, Stroke* stroke) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* ctrl = plugin->getControl();
    PageRef const& page = ctrl->getCurrentPage();
    Layer* layer = page->getSelectedLayer();

    std::string size;
    double thickness;
    int fillOpacity;
    bool filled;
    Color color;
    std::string lineStyle;
    ToolHandler* toolHandler;
    const char* tool;
    std::string allowUndoRedoAction;

    // Get attributes.
    lua_getfield(L, -1, "tool");
    lua_getfield(L, -2, "width");
    lua_getfield(L, -3, "color");
    lua_getfield(L, -4, "fill");
    lua_getfield(L, -5, "lineStyle");
    // stack now has following:
    //   -6 = table
    //   -5 = tool
    //   -4 = width
    //   -3 = color
    //   -2 = fill
    //   -1 = lineStyle

    tool = luaL_optstring(L, -5, "");  // We're gonna need the tool type.

    toolHandler = ctrl->getToolHandler();

    // TODO: (willnilges) Handle DrawingType?
    // TODO: (willnilges) Break out Eraser functionality into a new API call.

    // Set tool type
    if (strcmp("highlighter", tool) == 0) {
        stroke->setToolType(StrokeTool::HIGHLIGHTER);

        size = toolSizeToString(toolHandler->getHighlighterSize());
        thickness = toolHandler->getToolThickness(TOOL_HIGHLIGHTER)[toolSizeFromString(size)];

        fillOpacity = toolHandler->getHighlighterFill();
        filled = toolHandler->getHighlighterFillEnabled();

        Tool& tool = toolHandler->getTool(TOOL_HIGHLIGHTER);
        color = tool.getColor();
    } else {
        if (!(strcmp("pen", tool) == 0))
            g_warning("%s", FC(_F("Unknown stroke type: \"{1}\", defaulting to pen") % tool));

        stroke->setToolType(StrokeTool::PEN);

        size = toolSizeToString(toolHandler->getPenSize());
        thickness = toolHandler->getToolThickness(TOOL_PEN)[toolSizeFromString(size)];

        fillOpacity = toolHandler->getPenFill();
        filled = toolHandler->getPenFillEnabled();

        Tool& tool = toolHandler->getTool(TOOL_PEN);
        color = tool.getColor();
        lineStyle = StrokeStyle::formatStyle(tool.getLineStyle());
    }

    // Set width
    if (lua_isnumber(L, -4))  // Check if the width was provided
        stroke->setWidth(lua_tonumber(L, -4));
    else
        stroke->setWidth(thickness);

    // Set color
    if (lua_isinteger(L, -3))  // Check if the color was provided
        stroke->setColor(Color(lua_tointeger(L, -3)));
    else
        stroke->setColor(color);

    // Set fill
    if (lua_isinteger(L, -2))  // Check if fill settings were provided
        stroke->setFill(lua_tointeger(L, -2));
    else if (filled)
        stroke->setFill(fillOpacity);
    else
        stroke->setFill(-1);  // No fill

    // Set line style
    if (lua_isstring(L, -1))  // Check if line style settings were provided
        stroke->setLineStyle(StrokeStyle::parseStyle(lua_tostring(L, -1)));
    else
        stroke->setLineStyle(StrokeStyle::parseStyle(lineStyle.data()));

    // stack cleanup is needed as this is a helper function
    lua_pop(L, 5);  // Finally done with all that Lua data.

    // Add the stroke
    layer->addElement(stroke);
    return;
}

/**
 * Given a table containing a series of splines, draws a batch of strokes on the canvas.
 * Expects a table of tables containing eight coordinate pairs, along with attributes of the stroke.
 *
 * Required Arguments: splines
 * Optional Arguments: pressure, tool, width, color, fill, lineStyle
 *
 * If optional arguments are not provided, the specified tool settings are used.
 * If the tool is not provided, the current pen settings are used.
 * The only tools supported are Pen and Highlighter.
 *
 * The function expects 8 points per spline segment. Due to the nature of cubic
 * splines, you must pass your points in a repeating pattern:
 * startX, startY, ctrl1X, ctrl1Y, ctrl2X, ctrl2Y, endX, endY, startX, startY, ...
 *
 * The function checks that the length of the coordinate table is divisible by eight, and will throw
 * an error if it is not.
 *
 * Example: app.addSplines({
 *            ["splines"] = { -- The outer table is a table of strokes
 *                ["coordinates"] = { -- Each inner table is a coord stream that represents SplineSegments that can be
 * assembled into a stroke
 *                  [1] = 880.0, // Every eight coordinates (4 pairs) is a SplineSegment
 *                  [2] = 874.0,
 *                  [3] = 881.3295,
 *                  [4] = 851.5736,
 *                  [5] = 877.2915,
 *                  [6] = 828.2946,
 *                  [7] = 875.1697,
 *                  [8] = 806.0,
 *                  ... -- A spline can be made up of as many SplineSegments as is necessary.
 *                },
 *                -- Tool options are also specified per-stroke
 *                ["width"] = 1.4,
 *                ["color"] = 0xff0000,
 *                ["fill"] = 0,
 *                ["tool"] = "pen",
 *                ["lineStyle"] = "plain"
 *            },
 *            ["allowUndoRedoAction"] = "grouped", -- Each batch of splines can be grouped into one undo/redo action (or
 * "individual" or "none")
 *        })
 */

static int applib_addSplines(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* ctrl = plugin->getControl();
    std::vector<Element*> strokes;
    const char* allowUndoRedoAction;

    // Discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "splines");
    if (!lua_istable(L, -1))
        return luaL_error(L, "Missing spline table!");

    // stack now has following:
    //  1 = table arg
    // -1 = splines

    size_t numSplines = lua_rawlen(L, -1);
    for (size_t a = 1; a <= numSplines; a++) {
        std::vector<double> coordStream;
        // Get coordinates
        lua_pushnumber(L, a);
        lua_gettable(L, -2);                 // get current spline from splines table
        lua_getfield(L, -1, "coordinates");  // get coordinates of the current spline
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "Missing coordinate table!");
        }
        size_t numCoords = lua_rawlen(L, -1);
        for (size_t b = 1; b <= numCoords; b++) {
            lua_pushnumber(L, b);
            lua_gettable(L, -2);  // get current coordinate from coordinates
            double point = lua_tonumber(L, -1);
            coordStream.push_back(point);  // Each segment is going to have multiples of 8 points.
            lua_pop(L, 1);                 // cleanup fetched coordinate
        }
        // pop value + copy of key, leaving original key
        lua_pop(L, 1);  // cleanup coordinates table
        // Handle those points
        // Check if the list is divisible by 8.
        if (coordStream.size() % 8 != 0) {
            return luaL_error(L, "Point table incomplete!");
        }

        if (coordStream.empty()) {
            lua_pop(L, 1);
            continue;
        }

        const Point firstKnot(coordStream[0], coordStream[1]);
        // Reserve the space for the right number of segments
        std::shared_ptr<Spline> spline = std::make_shared<Spline>(firstKnot, coordStream.size() / 8);

        // Now take that gigantic list of splines and create SplineSegments out of them.

        for (size_t i = 0; i < coordStream.size(); i += 8) {
            Point ctrl1 = Point(coordStream[i + 2], coordStream[i + 3]);
            Point ctrl2 = Point(coordStream.at(i + 4), coordStream[i + 5]);
            Point end = Point(coordStream[i + 6], coordStream[i + 7]);
            spline->addCubicSegment(ctrl1, ctrl2, end);
        }

        if (spline->nbSegments() > 0) {
            // Finish building the Stroke and apply it to the layer.
            Stroke* stroke = new Stroke();
            stroke->setPath(std::move(spline));
            addStrokeHelper(L, stroke);
            strokes.push_back(stroke);
        } else {
            g_warning("Stroke has no spline segments. Discarding.");
        }
        // Onto the next stroke
        lua_pop(L, 1);  // cleanup current spline
    }

    // Stack is now the same as it was on entry to this function
    lua_pop(L, 1);  // cleanup splines table

    // stack now has following:
    //  1 = table arg

    // Check how the user wants to handle undoing
    lua_getfield(L, 1, "allowUndoRedoAction");
    allowUndoRedoAction = luaL_optstring(L, -1, "grouped");
    lua_pop(L, 1);
    handleUndoRedoActionHelper(L, ctrl, allowUndoRedoAction, strokes);
    return 0;
}

/**
 * Given a table of sets of points, draws a batch of strokes on the canvas.
 * Expects three tables of equal length: one for X, one for Y, and one for
 * stroke pressure, along with attributes of the stroke. Each stroke has
 * attributes handled individually.
 *
 * Required Arguments: X, Y
 * Optional Arguments: pressure, tool, width, color, fill, lineStyle
 *
 * If optional arguments are not provided, the specified tool settings are used.
 * If the tool is not provided, the current pen settings are used.
 * The only tools supported are Pen and Highlighter.
 *
 * The function checks for consistency among table lengths, and throws an
 * error if there is a discrepancy
 *
 * Example:
 *
 * app.addStrokes({
 *     ["strokes"] = { -- The outer table is a table of strokes
 *         {   -- Inside a stroke are three tables of equivalent length that represent a series of points
 *             ["x"]        = { [1] = 110.0, [2] = 120.0, [3] = 130.0, ... },
 *             ["y"]        = { [1] = 200.0, [2] = 205.0, [3] = 210.0, ... },
 *             ["pressure"] = { [1] = 0.8,   [2] = 0.9,   [3] = 1.1, ... },
 *             -- Each stroke has individually handled options
 *             ["tool"] = "pen",
 *             ["width"] = 3.8,
 *             ["color"] = 0xa000f0,
 *             ["fill"] = 0,
 *             ["lineStyle"] = "solid",
 *         },
 *         {
 *             ["x"]        = { [1] = 310.0, [2] = 320.0, [3] = 330.0, ... },
 *             ["y"]        = { [1] = 300.0, [2] = 305.0, [3] = 310.0, ... },
 *             ["pressure"] = { [1] = 3.0,   [2] = 3.0,   [3] = 3.0, ... },
 *             ["tool"] = "pen",
 *             ["width"] = 1.21,
 *             ["color"] = 0x808000,
 *             ["fill"] = 0,
 *             ["lineStyle"] = "solid",
 *         },
 *         {
 *             ["x"]        = { [1] = 27.0,  [2] = 28.0,  [3] = 30.0, ... },
 *             ["y"]        = { [1] = 100.0, [2] = 102.3, [3] = 102.5, ... },
 *             ["pressure"] = { [1] = 1.0,   [2] = 1.0,   [3] = 1.0, ... },
 *             ["tool"] = "pen",
 *             ["width"] = 1.0,
 *             ["color"] = 0x00aaaa,
 *             ["fill"] = 0,
 *             ["lineStyle"] = "dashdot",
 *         },
 *     },
 *     ["allowUndoRedoAction"] = "grouped", -- Each batch of strokes can be grouped into one undo/redo action (or
 * "individual" or "none")
 * })
 */
static int applib_addStrokes(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* ctrl = plugin->getControl();
    std::vector<Element*> strokes;
    const char* allowUndoRedoAction;

    // Discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "strokes");
    if (!lua_istable(L, -1))
        return luaL_error(L, "Missing stroke table!");

    // stack now has following:
    //  1 = table arg
    // -1 = strokes

    size_t numStrokes = lua_rawlen(L, -1);
    for (size_t a = 1; a <= numStrokes; a++) {
        std::vector<double> xStream;
        std::vector<double> yStream;
        std::vector<double> pressureStream;

        // Fetch table of X values from the Lua stack
        lua_pushnumber(L, a);
        lua_gettable(L, -2);  // get current stroke

        lua_getfield(L, -1, "x");  // get x array of current stroke
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "Missing X-Coordinate table!");
        }
        size_t xPoints = lua_rawlen(L, -1);
        for (size_t b = 1; b <= xPoints; b++) {
            lua_pushnumber(L, b);
            lua_gettable(L, -2);  // get current x-Coordinate
            double value = lua_tonumber(L, -1);
            xStream.push_back(value);
            lua_pop(L, 1);  // cleanup x-Coordinate
        }
        lua_pop(L, 1);  // cleanup x array

        // Fetch table of Y values form the Lua stack
        lua_getfield(L, -1, "y");  // get y array of current stroke
        if (!lua_istable(L, -1)) {
            return luaL_error(L, "Missing Y-Coordinate table!");
        }
        size_t yPoints = lua_rawlen(L, -1);
        for (size_t b = 1; b <= yPoints; b++) {
            lua_pushnumber(L, b);
            lua_gettable(L, -2);  // get current y-Coordinate
            double value = lua_tonumber(L, -1);
            yStream.push_back(value);
            lua_pop(L, 1);  // cleanup y-Coordinate
        }
        lua_pop(L, 1);  // cleanup y array

        // Fetch table of pressure values from the Lua stack
        lua_getfield(L, -1, "pressure");
        if (lua_istable(L, -1)) {
            size_t pressurePoints = lua_rawlen(L, -1);
            for (size_t b = 1; b <= pressurePoints; b++) {
                lua_pushnumber(L, b);
                lua_gettable(L, -2);  // get current pressure
                double value = lua_tonumber(L, -1);
                pressureStream.push_back(value);
                lua_pop(L, 1);  // cleanup pressure
            }
        } else {
            g_warning("Missing pressure table. Assuming NO_PRESSURE.");
        }

        lua_pop(L, 1);  // cleanup pressure array

        // Handle those points
        // Make sure all vectors are the same length.
        if (xStream.size() != yStream.size()) {
            return luaL_error(L, "X and Y vectors are not equal length!");
        }
        if (xStream.size() != pressureStream.size() && pressureStream.size() > 0) {
            return luaL_error(L, "Pressure vector is not equal length!");
        }

        // Check and make sure there's enough points (need at least 2)
        if (xStream.size() < 2) {
            g_warning("Stroke shorter than two points. Discarding. (Has %ld/2)", xStream.size());
            return 1;
        }

        const Point firstKnot(xStream.front(), yStream.front(),
                              pressureStream.size() > 0 ? pressureStream.front() : Point::NO_PRESSURE);
        // Reserve space for xStream.size() - 1 segments.
        std::shared_ptr<PiecewiseLinearPath> path =
                std::make_shared<PiecewiseLinearPath>(firstKnot, xStream.size() - 1);
        // Add points to the stroke. Include pressure, if it exists.
        if (pressureStream.size() > 0) {
            for (long unsigned int i = 1; i < xStream.size(); i++) {
                Point myPoint = Point(xStream.at(i), yStream.at(i), pressureStream.at(i));
                path->addLineSegmentTo(myPoint);
            }
        } else {
            for (long unsigned int i = 1; i < xStream.size(); i++) {
                Point myPoint = Point(xStream.at(i), yStream.at(i), Point::NO_PRESSURE);
                path->addLineSegmentTo(myPoint);
            }
        }

        // Finish building the Stroke and apply it to the layer.
        Stroke* stroke = new Stroke();
        stroke->setPath(std::move(path));
        addStrokeHelper(L, stroke);
        strokes.push_back(stroke);
        // Onto the next stroke
        lua_pop(L, 1);  // cleanup stroke table
    }

    // stack now has following:
    //  1 = table arg
    // -1 = strokes

    // Check how the user wants to handle undoing
    lua_getfield(L, 1, "allowUndoRedoAction");
    allowUndoRedoAction = luaL_optstring(L, -1, "grouped");
    if (strcmp("grouped", allowUndoRedoAction) == 0) {
        PageRef const& page = ctrl->getCurrentPage();
        Layer* layer = page->getSelectedLayer();
        UndoRedoHandler* undo = ctrl->getUndoRedoHandler();
        undo->addUndoAction(std::make_unique<InsertsUndoAction>(page, layer, strokes));
    } else if (strcmp("individual", allowUndoRedoAction) == 0) {
        PageRef const& page = ctrl->getCurrentPage();
        Layer* layer = page->getSelectedLayer();
        UndoRedoHandler* undo = ctrl->getUndoRedoHandler();
        for (Element* element: strokes) undo->addUndoAction(std::make_unique<InsertUndoAction>(page, layer, element));
    } else if (strcmp("none", allowUndoRedoAction) == 0)
        g_warning("Not allowing undo/redo action.");
    else
        return luaL_error(L, "Unrecognized undo/redo option: %s", allowUndoRedoAction);

    return 0;
}

/**
 * Adds textboxes as specified to the current layer.
 *
 * Global parameters:
 *   - texts table: array of text-parameter-tables
 *   - allowUndoRedoAction string: Decides how the change gets introduced into the undoRedo action list "individual",
 * "grouped" or "none"
 *
 * Parameters per textbox:
 *   - text string: content of the textbox (required)
 *   - font table {name string, size number} (default: currently configured font/size from the settings)
 *   - color integer: RGB hex code for the text-color (default: color of text tool)
 *   - x number: x-position of the box (upper left corner) (required)
 *   - y number: y-position of the box (upper left corner) (required)
 *
 * Example:
 *
 * app.addTexts{texts={
 *   {
 *     text="Hello World",
 *     font={name="Noto Sans Mono Medium", size=8.0},
 *     color=0x1259b9,
 *     x = 50.0,
 *     y = 50.0,
 *   },
 *   {
 *     text="Testing",
 *     font={name="Noto Sans Mono Medium", size=8.0},
 *     color=0x0,
 *     x = 150.0,
 *     y = 50.0,
 *   },
 * }
 */

static int applib_addTexts(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    PageRef const& page = control->getCurrentPage();
    Layer* layer = page->getSelectedLayer();
    Settings* settings = control->getSettings();

    std::vector<Element*> texts;

    // Discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "texts");
    if (!lua_istable(L, -1)) {
        return luaL_error(L, "Missing text table!");
    }

    // stack now has following:
    //  1 = table arg
    // -1 = texts array

    // get default color
    ToolHandler* toolHandler = control->getToolHandler();
    Tool& tool = toolHandler->getTool(TOOL_TEXT);
    Color default_color = tool.getColor();
    // default font
    XojFont& default_font = settings->getFont();

    size_t numTexts = lua_rawlen(L, -1);
    for (size_t a = 1; a <= numTexts; a++) {
        Text* t = new Text();

        // Fetch table of X values from the Lua stack
        lua_pushnumber(L, a);
        lua_gettable(L, -2);  // get current text
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_getfield(L, -1, "text");

        // handle font table
        lua_getfield(L, -2, "font");  // {name="", size=0}
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "name");
            lua_getfield(L, -2, "size");
        } else if (lua_isnil(L, -1)) {
            // push two dummy values if font is unset/no table
            lua_pushnil(L);
            lua_pushnil(L);
        } else {
            return luaL_error(L, "'font' value must be a table!");
        }

        lua_getfield(L, -5, "color");
        lua_getfield(L, -6, "x");
        lua_getfield(L, -7, "y");

        // stack now has following:
        //    1 = global params table
        //   -9 = texts array
        //   -8 = current text-params table
        //   -7 = text
        //   -6 = font-table
        //   -5 = fontname
        //   -4 = fontsize
        //   -3 = color
        //   -2 = x
        //   -1 = y

        if (!lua_isstring(L, -7)) {
            return luaL_error(L, "Missing text!/'text' must be a string");
        }
        t->setText(lua_tostring(L, -7));

        XojFont font{};
        font.setName(luaL_optstring(L, -5, default_font.getName().c_str()));
        font.setSize(luaL_optnumber(L, -4, default_font.getSize()));
        t->setFont(font);

        if (lua_isinteger(L, -3)) {  // Check if the color was provided
            uint32_t color = lua_tointeger(L, -3);
            if (color > 0xffffff) {
                return luaL_error(L, "Color 0x%x is no valid RGB color. ", color);
            }
            t->setColor(Color(color));
        } else if (lua_isnil(L, -3)) {
            t->setColor(default_color);
        } else {
            return luaL_error(L, "'color' must be an integer/hex-code or unset");
        }

        if (!lua_isnumber(L, -2)) {  // Check if x was provided
            return luaL_error(L, "Missing X-Coordinate!/must be a number");
        }
        t->setX(lua_tonumber(L, -2));

        if (!lua_isnumber(L, -1)) {  // Check if y was provided
            return luaL_error(L, "Missing Y-Coordinate!/must be a number");
        }
        t->setY(lua_tonumber(L, -1));

        lua_pop(L, 8);  // remove values read out from the text table + text-table itself

        // Finish building the Text and apply it to the layer.
        layer->addElement(t);
        texts.push_back(t);
        // Onto the next text
    }

    // stack now has following:
    //  1 = table arg
    // -1 = texts array


    lua_getfield(L, 1, "allowUndoRedoAction");
    const char* allowUndoRedoAction = luaL_optstring(L, -1, "grouped");
    lua_pop(L, 1);
    handleUndoRedoActionHelper(L, control, allowUndoRedoAction, texts);

    return 0;
}

/*
 * Returns a list of lua table of the texts (from current selection / current layer).
 * Is mostly inverse to app.addTexts (except getTexts will also retrieve the width/height of the textbox)
 *
 * Required argument: type ("selection" or "layer")
 *
 * Example: local texts = app.getTexts("layer")
 *
 * possible return value:
 * {
 *   {
 *     text = "Hello World",
 *     font = {
 *             name = "Noto Sans Mono Medium",
 *             size = 8.0,
 *            },
 *     color = 0x1259b9,
 *     x = 127.0,
 *     y = 70.0,
 *     width = 55.0,
 *     height = 23.0,
 *   },
 *   {
 *     text = "Testing",
 *     font = {
 *             name = "Noto Sans Mono Medium",
 *             size = 8.0,
 *            },
 *     color = 0x0,
 *     x = 150.0,,
 *     y = 70.0,
 *     width = 55.0,
 *     height = 23.0,
 *   },
 * }
 *
 */

static int applib_getTexts(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    std::string type = luaL_checkstring(L, 1);
    std::vector<Element*> elements = {};
    Control* control = plugin->getControl();

    // Discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TSTRING);

    if (type == "layer") {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (sel) {
            control->clearSelection();  // otherwise texts in the selection won't be recognized
        }
        elements = control->getCurrentPage()->getSelectedLayer()->getElements();
    } else if (type == "selection") {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (sel) {
            elements = sel->getElements();
        } else {
            return luaL_error(L, "There is no selection! ");
        }
    } else {
        return luaL_error(L, "Unknown argument: %s", type.c_str());
    }

    lua_newtable(L);  // create table of the elements
    int currTextNo = 0;

    // stack now has following:
    //  1 = type (string)
    // -1 = table of texts (to be returned)

    for (Element* e: elements) {
        if (e->getType() == ELEMENT_TEXT) {
            auto* t = static_cast<Text*>(e);
            lua_pushnumber(L, ++currTextNo);  // index for later (settable)
            lua_newtable(L);                  // create text table

            // stack now has following:
            //  1 = type (string)
            // -3 = table of texts (to be returned)
            // -2 = index of the current text
            // -1 = current text table

            lua_pushstring(L, t->getText().c_str());
            lua_setfield(L, -2, "text");  // add text to text element

            lua_newtable(L);  // font table to stack
            lua_pushstring(L, t->getFontName().c_str());
            lua_setfield(L, -2, "name");  // add font to text
            lua_pushnumber(L, t->getFontSize());
            lua_setfield(L, -2, "size");  // add size to text
            lua_setfield(L, -2, "font");  // insert font-table to text element

            lua_pushinteger(L, int(uint32_t(t->getColor())));
            lua_setfield(L, -2, "color");  // add color to text

            lua_pushnumber(L, t->getX());
            lua_setfield(L, -2, "x");  // add x coordindate to text

            lua_pushnumber(L, t->getY());
            lua_setfield(L, -2, "y");  // add y coordinate to text

            lua_pushnumber(L, t->getElementWidth());
            lua_setfield(L, -2, "width");  // add width to text

            lua_pushnumber(L, t->getElementHeight());
            lua_setfield(L, -2, "height");  // add height to text

            lua_settable(L, -3);  // add text to elements
        }
    }
    return 1;
}

/**
 * Puts a Lua Table of the Strokes (from the selection tool / selected layer) onto the stack.
 * Is inverse to app.addStrokes
 *
 * Required argument: type ("selection" or "layer")
 *
 * Example: local strokes = app.getStrokes("selection")
 *
 * possible return value:
 * {
 *         {   -- Inside a stroke are three tables of equivalent length that represent a series of points
 *             ["x"]        = { [1] = 110.0, [2] = 120.0, [3] = 130.0, ... },
 *             ["y"]        = { [1] = 200.0, [2] = 205.0, [3] = 210.0, ... },
 *             -- pressure is only present if pressure is set -> pressure member might be nil
 *             ["pressure"] = { [1] = 0.8,   [2] = 0.9,   [3] = 1.1, ... },
 *             -- Each stroke has individually handled options
 *             ["tool"] = "pen",
 *             ["width"] = 3.8,
 *             ["color"] = 0xa000f0,
 *             ["fill"] = 0,
 *             ["lineStyle"] = "plain",
 *         },
 *         {
 *             ["x"]         = {207, 207.5, 315.2, 315.29, 207.5844},
 *             ["y"]         = {108, 167.4, 167.4, 108.70, 108.7094},
 *             ["tool"]      = "pen",
 *             ["width"]     = 0.85,
 *             ["color"]     = 16744448,
 *             ["fill"]      = -1,
 *             ["lineStyle"] = "plain",
 *         },
 *         {
 *             ["x"]         = {387.60, 387.6042, 500.879, 500.87, 387.604},
 *             ["y"]         = {153.14, 215.8661, 215.866, 153.14, 153.148},
 *             ["tool"]      = "pen",
 *             ["width"]     = 0.85,
 *             ["color"]     = 16744448,
 *             ["fill"]      = -1,
 *             ["lineStyle"] = "plain",
 *         },
 * }
 */
static int applib_getStrokes(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    std::string type = luaL_checkstring(L, 1);
    std::vector<Element*> elements = {};
    Control* control = plugin->getControl();

    // Discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TSTRING);

    if (type == "layer") {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (sel) {
            control->clearSelection();  // otherwise strokes in the selection won't be recognized
        }
        elements = control->getCurrentPage()->getSelectedLayer()->getElements();
    } else if (type == "selection") {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (sel) {
            elements = sel->getElements();
        } else {
            return luaL_error(L, "There is no selection! ");
        }
    } else {
        return luaL_error(L, "Unknown argument: %s", type.c_str());
    }

    lua_newtable(L);  // create table of the elements
    int currStrokeNo = 0;
    int currPointNo = 0;

    // stack now has following:
    //  1 = type (string)
    // -1 = table of strokes (to be returned)

    for (Element* e: elements) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = static_cast<Stroke*>(e);
            lua_pushnumber(L, ++currStrokeNo);  // index for later (settable)
            lua_newtable(L);                    // create stroke table

            // stack now has following:
            //  1 = type (string)
            // -3 = table of strokes (to be returned)
            // -2 = index of the current stroke
            // -1 = current stroke

            auto& pts = s->getPath().getData();

            lua_newtable(L);  // create table of x-coordinates
            for (auto p: pts) {
                lua_pushnumber(L, ++currPointNo);  // key
                lua_pushnumber(L, p.x);            // value
                lua_settable(L, -3);               // insert
            }
            lua_setfield(L, -2, "x");  // add x-coordinates to stroke
            currPointNo = 0;

            lua_newtable(L);  // create table for y-coordinates
            for (auto p: pts) {
                lua_pushnumber(L, ++currPointNo);  // key
                lua_pushnumber(L, p.y);            // value
                lua_settable(L, -3);               // insert
            }
            lua_setfield(L, -2, "y");  // add y-coordinates to stroke
            currPointNo = 0;

            if (s->hasPressure()) {
                lua_newtable(L);  // create table for pressures
                for (auto p: pts) {
                    lua_pushnumber(L, ++currPointNo);  // key
                    lua_pushnumber(L, p.z);            // value
                    lua_settable(L, -3);               // insert
                }
                lua_setfield(L, -2, "pressure");  // add pressures to stroke
                currPointNo = 0;
            }

            // stack now has following:
            //  1 = type (string)
            // -3 = table of strokes (to be returned)
            // -2 = index of the current stroke
            // -1 = current stroke

            StrokeTool tool = s->getToolType();
            if (tool == StrokeTool::PEN) {
                lua_pushstring(L, "pen");
            } else if (tool == StrokeTool::ERASER) {
                lua_pushstring(L, "eraser");
            } else if (tool == StrokeTool::HIGHLIGHTER) {
                lua_pushstring(L, "highlighter");
            } else {
                return luaL_error(L, "Unknown StrokeTool::Value.");
            }
            lua_setfield(L, -2, "tool");  // add tool to stroke

            lua_pushnumber(L, s->getWidth());
            lua_setfield(L, -2, "width");  // add width to stroke

            lua_pushinteger(L, int(uint32_t(s->getColor())));
            lua_setfield(L, -2, "color");  // add color to stroke

            lua_pushinteger(L, s->getFill());
            lua_setfield(L, -2, "fill");  // add fill to stroke

            lua_pushstring(L, StrokeStyle::formatStyle(s->getLineStyle()).c_str());
            lua_setfield(L, -2, "lineStyle");  // add linestyle to stroke

            lua_settable(L, -3);  // add stroke to returned table
        }
    }
    return 1;
}

/**
 * Notifies program of any updates to the working document caused
 * by the API.
 *
 * Example: app.refreshPage()
 */
static int applib_refreshPage(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* ctrl = plugin->getControl();
    PageRef const& page = ctrl->getCurrentPage();
    if (page)
        page->firePageChanged();
    else
        return luaL_error(L, "Called applib_refreshPage, but there is no current page.");
    return 0;
}

/**
 * Change page background of current page
 *
 * Example: app.changeCurrentPageBackground("graph")
 * changes the page background of the current page to graph paper
 */
static int applib_changeCurrentPageBackground(lua_State* L) {
    PageType pt;
    pt.format = PageTypeHandler::getPageTypeFormatForString(luaL_checkstring(L, 1));
    pt.config = luaL_optstring(L, 2, "");

    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* ctrl = plugin->getControl();
    PageBackgroundChangeController* pageBgCtrl = ctrl->getPageBackgroundChangeController();
    pageBgCtrl->changeCurrentPageBackground(pt);

    return 0;
}

/**
 * Change color of a specified tool or of the current tool
 *
 * Example 1: app.changeToolColor({["color"] = 0xff00ff, ["tool"] = "PEN"})
 * changes the color of the pen tool to violet without applying this change to the current selection
 *
 * Example 2: app.changeToolColor({["color"] = 0xff0000, ["selection"] = true })
 * changes the color of the current tool to red and also applies it to the current selection if there is one
 */
static int applib_changeToolColor(lua_State* L) {

    // discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "selection"); /* either true or false, for changing selection color
                                       defaults to false*/
    lua_getfield(L, 1, "tool");      /* "pen", "highlighter", "text"
                                      "select_rect", "select_object", "select_region"
                                      if omitted, current Tool is used */
    lua_getfield(L, 1, "color");     // an RGB hex code defining the color
    // stack now has following:
    //    1 = {["color"] = 0xff00ff, ["tool"] = "PEN", ["selection"] = true}
    //   -3 = true
    //   -2 = "pen"
    //   -1 = 0xff0077

    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* ctrl = plugin->getControl();
    ToolHandler* toolHandler = ctrl->getToolHandler();

    bool selection = false;
    if (lua_isboolean(L, -3)) {
        selection = lua_toboolean(L, -3);
    } else if (!lua_isnil(L, -3)) {
        return luaL_error(L, ""
                             "selection"
                             " key should be a boolean value (or nil)");
    }

    ToolType toolType = toolHandler->getToolType();
    const char* toolStr = luaL_optstring(L, -2, nullptr);
    if (toolStr != nullptr) {
        toolType = toolTypeFromString(StringUtils::toLowerCase(toolStr));
    }

    if (toolType == TOOL_NONE) {
        lua_pop(L, 3);
        return luaL_error(L, "tool \"%s\" is not valid or no tool has been selected",
                          toolTypeToString(toolType).c_str());
    }

    uint32_t color = 0x000000;
    if (lua_isinteger(L, -1)) {
        color = as_unsigned(lua_tointeger(L, -1));
        if (color > 0xffffff) {
            return luaL_error(L, "Color 0x%x is no valid RGB color. ", color);
        }
    } else if (!lua_isnil(L, -1)) {
        return luaL_error(L, " "
                             "color"
                             " key should be an RGB hex code in the form 0xRRGGBB (or nil)");
    }

    Tool& tool = toolHandler->getTool(toolType);

    if (tool.hasCapability(TOOL_CAP_COLOR)) {
        tool.setColor(Color(color));
        ctrl->toolColorChanged();
        if (selection)
            ctrl->changeColorOfSelection();
    } else {
        return luaL_error(L, "tool \"%s\" has no color capability", toolTypeToString(toolType).c_str());
    }

    return 0;
}

/*
 * Select Background Pdf Page for Current Page
 * First argument is an integer (page number) and the second argument is a boolean (isRelative)
 * specifying whether the page number is relative to the current pdf page or absolute
 *
 * Example 1: app.changeBackgroundPdfPageNr(1, true)
 * changes the pdf page to the next one (relative mode)
 *
 * Example 2: app.changeBackgroundPdfPageNr(7, false)
 * changes the page background to the 7th pdf page (absolute mode)
 */
static int applib_changeBackgroundPdfPageNr(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);

    size_t nr = luaL_checkinteger(L, 1);
    bool relative = true;
    if (lua_isboolean(L, 2)) {
        relative = lua_toboolean(L, 2);
    }

    Control* control = plugin->getControl();
    Document* doc = control->getDocument();
    PageRef const& page = control->getCurrentPage();

    if (!page) {
        return luaL_error(L, "No page!");
    }

    size_t selected = nr - 1;
    if (relative) {
        bool isPdf = page->getBackgroundType().isPdfPage();
        if (isPdf) {
            selected = page->getPdfPageNr() + nr;
        } else {
            return luaL_error(L, "Current page has no pdf background, cannot use relative mode!");
        }
    }
    if (selected < doc->getPdfPageCount()) {
        // no need to set a type, if we set the page number the type is also set
        page->setBackgroundPdfPageNr(selected);

        XojPdfPageSPtr p = doc->getPdfPage(selected);
        page->setSize(p->getWidth(), p->getHeight());
    } else {
        return luaL_error(L, "Pdf page number %d does not exist!", selected + 1);
    }

    return 0;
}

/*
 * pushes an rectangle (width, height, x, y) to the lua stack as a new table
 * in most cases you'll want to do a
 * lua_newtable(L);  // create rectangle table
 * before and a
 * lua_setfield(L, -2, "rectangleTable");  // add rectangle table to other table
 * after this function
 */
static void pushRectangleHelper(lua_State* L, xoj::util::Rectangle<double> rect) {
    lua_pushnumber(L, rect.width);
    lua_setfield(L, -2, "width");
    lua_pushnumber(L, rect.height);
    lua_setfield(L, -2, "height");
    lua_pushnumber(L, rect.x);
    lua_setfield(L, -2, "x");
    lua_pushnumber(L, rect.y);
    lua_setfield(L, -2, "y");
}
/*
 * Returns a table encoding all info on the chosen tool (active, pen, highlighter, eraser or text)
 * in a Lua table of one of the following shapes
 *
 * for pen:
 * {
 *   "size" = string
 *   "color" = integer
 *   "filled" = bool
 *   "fillOpacity" = integer (0 to 255)
 *   "drawingType" = string
 *   "lineStyle" = string
 * }
 *
 * See /src/control/ToolEnums.cpp for possible values of "size".
 *
 * for text:
 * {
 *   "font" = {
 *     name = string.
 *     size = number
 *   }
 *   "color" = integer
 * }
 *
 * for active tool:
 * {
 *   "type" = string
 *   "size" = {
 *     name = string.
 *     value = number
 *   }
 *   "color" = integer
 *   "fillOpacity" = integer (0 to 255)
 *   "drawingType" = string
 *   "lineStyle" = string
 *   "thickness" = number
 * }
 *
 * See /src/control/ToolEnums.cpp for possible values of "type", "size", "drawingType" and "lineStyle".
 *
 * for eraser:
 * {
 *   "type" = string
 *   "size" = string
 * }
 *
 * See /src/control/ToolEnums.cpp for possible values of "type" and "size".
 *
 * for highlighter:
 * {
 *   "size" = string
 *   "color" = integer
 *   "filled" = bool
 *   "fillOpacity" = integer (0 to 255)
 *   "drawingType" = string
 * }
 *
 * See /src/control/ToolEnums.cpp for possible values of "size".
 *
 * for seiection:
 * {
 *   -- bounding box as drawn in the UI (includes padding on all sides)
 *   "boundingBox" = {
 *      "width"  = number
 *      "height" = number
 *      "x"      = number
 *      "y"      = number
 *   }
 *   -- same as "boundingBox" but the state before any transformation was applied
 *   "originalBounds" = {
 *      "width"  = number
 *      "height" = number
 *      "x"      = number
 *      "y"      = number
 *   }
 *   -- bounds used for snapping (doesn't include padding and doesn't account to line width)
 *   -- for more information see https://github.com/xournalpp/xournalpp/pull/4359#issuecomment-1304395011
 *   "snappedBounds" = {
 *      "width"  = number
 *      "height" = number
 *      "x"      = number
 *      "y"      = number
 *   }
 *   "rotation" = number
 *   "isRotationSupported" = bool
 * }
 *
 * Example 1: local penInfo = app.getToolInfo("pen")
 *            local size = penInfo["size"]
 *            local opacity = penInfo["fillOpacity"]
 * *
 * Example 2: local font = app.getToolInfo("text")["font"]
 *            local fontname = font["name"]
 *            local fontsize = font["size"]
 *
 * Example 3: local color = app.getToolInfo("text")["color"]
 *            local red = color >> 16 & 0xff
 *            local green = color >> 8 & 0xff
 *            local blue = color & 0xff
 *
 * Example 4: local activeToolInfo = app.getToolInfo("active")
 *            local thickness = activeToolInfo["thickness"]
 *            local drawingType = activeToolInfo["drawingType"]
 *
 * Example 5: local eraserInfo = app.getToolInfo("eraser")
 *            local type = eraserInfo["type"]
 *            local size = eraserInfo["size"]
 *            local sizeName = size["name"]
 *            local thickness = size["value"]
 *
 * Example 6: local highlighterInfo = app.getToolInfo("highlighter")
 *            local sizeName = highlighterInfo["size"]["name"]
 *            local opacity = highlighterInfo["fillOpacity"]
 *
 * Example 7: local selectionInfo = app.getToolInfo("selection")
 *            local rotation = selectionInfo["rotation"]
 *            local boundingX = selectionInfo["boundingBox"]["x"]
 *            local snappedBoundsWidth = selectionInfo["snappedBounds"]["width"]
 */

static int applib_getToolInfo(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    ToolHandler* toolHandler = control->getToolHandler();

    // discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TSTRING);

    const char* mode = luaL_checkstring(L, -1);
    lua_newtable(L);  // return table

    // stack now has following:
    //    1 = mode
    //   -1 = table to be returned

    if (strcmp(mode, "active") == 0) {
        std::string toolType = toolTypeToString(toolHandler->getToolType());

        std::string toolSize = toolSizeToString(toolHandler->getSize());
        double thickness = toolHandler->getThickness();

        Color color = toolHandler->getColor();
        int fillOpacity = toolHandler->getFill();
        std::string drawingType = drawingTypeToString(toolHandler->getDrawingType());
        std::string lineStyle = StrokeStyle::formatStyle(toolHandler->getLineStyle());


        lua_pushstring(L, toolType.c_str());  // value
        lua_setfield(L, -2, "type");          // insert

        lua_newtable(L);  // beginning of "size" table

        lua_pushstring(L, toolSize.c_str());  // value
        lua_setfield(L, -2, "name");          // insert

        lua_pushnumber(L, thickness);  // value
        lua_setfield(L, -2, "value");  // insert

        lua_setfield(L, -2, "size");  // end of "size" table

        lua_pushinteger(L, int(uint32_t(color)));  // value
        lua_setfield(L, -2, "color");              // insert

        lua_pushinteger(L, fillOpacity);     // value
        lua_setfield(L, -2, "fillOpacity");  // insert

        lua_pushstring(L, drawingType.c_str());  // value
        lua_setfield(L, -2, "drawingType");      // insert

        lua_pushstring(L, lineStyle.c_str());  // value
        lua_setfield(L, -2, "lineStyle");      // insert
    } else if (strcmp(mode, "pen") == 0) {
        std::string size = toolSizeToString(toolHandler->getPenSize());
        double thickness = toolHandler->getToolThickness(TOOL_PEN)[toolSizeFromString(size)];

        int fillOpacity = toolHandler->getPenFill();
        bool filled = toolHandler->getPenFillEnabled();

        Tool& tool = toolHandler->getTool(TOOL_PEN);
        Color color = tool.getColor();
        std::string drawingType = drawingTypeToString(tool.getDrawingType());
        std::string lineStyle = StrokeStyle::formatStyle(tool.getLineStyle());

        lua_newtable(L);  // beginning of "size" table

        lua_pushstring(L, size.c_str());  // value
        lua_setfield(L, -2, "name");      // insert

        lua_pushnumber(L, thickness);  // value
        lua_setfield(L, -2, "value");  // insert

        lua_setfield(L, -2, "size");  // end of "size" table

        lua_pushinteger(L, int(uint32_t(color)));  // value
        lua_setfield(L, -2, "color");              // insert

        lua_pushstring(L, drawingType.c_str());  // value
        lua_setfield(L, -2, "drawingType");      // insert

        lua_pushstring(L, lineStyle.c_str());  // value
        lua_setfield(L, -2, "lineStyle");      // insert

        lua_pushboolean(L, filled);     // value
        lua_setfield(L, -2, "filled");  // insert

        lua_pushinteger(L, fillOpacity);     // value
        lua_setfield(L, -2, "fillOpacity");  // insert
    } else if (strcmp(mode, "highlighter") == 0) {
        std::string size = toolSizeToString(toolHandler->getHighlighterSize());
        double thickness = toolHandler->getToolThickness(TOOL_HIGHLIGHTER)[toolSizeFromString(size)];

        int fillOpacity = toolHandler->getHighlighterFill();
        bool filled = toolHandler->getHighlighterFillEnabled();

        Tool& tool = toolHandler->getTool(TOOL_HIGHLIGHTER);
        Color color = tool.getColor();
        std::string drawingType = drawingTypeToString(tool.getDrawingType());

        lua_newtable(L);  // beginning of "size" table

        lua_pushstring(L, size.c_str());  // value
        lua_setfield(L, -2, "name");      // insert

        lua_pushnumber(L, thickness);  // value
        lua_setfield(L, -2, "value");  // insert

        lua_setfield(L, -2, "size");  // end of "size" table

        lua_pushinteger(L, int(uint32_t(color)));  // value
        lua_setfield(L, -2, "color");              // insert

        lua_pushstring(L, drawingType.c_str());  // value
        lua_setfield(L, -2, "drawingType");      // insert

        lua_pushboolean(L, filled);     // value
        lua_setfield(L, -2, "filled");  // insert

        lua_pushinteger(L, fillOpacity);     // value
        lua_setfield(L, -2, "fillOpacity");  // insert
    } else if (strcmp(mode, "eraser") == 0) {
        std::string type = eraserTypeToString(toolHandler->getEraserType());

        std::string size = toolSizeToString(toolHandler->getEraserSize());
        double thickness = toolHandler->getToolThickness(ToolType::TOOL_ERASER)[toolSizeFromString(size)];

        lua_pushstring(L, type.c_str());  // value
        lua_setfield(L, -2, "type");      // insert

        lua_newtable(L);  // beginning of "size" table

        lua_pushstring(L, size.c_str());  // value
        lua_setfield(L, -2, "name");      // insert

        lua_pushnumber(L, thickness);  // value
        lua_setfield(L, -2, "value");  // insert

        lua_setfield(L, -2, "size");  // end of "size" table
    } else if (strcmp(mode, "text") == 0) {
        Settings* settings = control->getSettings();
        XojFont& font = settings->getFont();
        std::string fontname = font.getName();
        double size = font.getSize();

        Tool& tool = toolHandler->getTool(TOOL_TEXT);
        Color color = tool.getColor();

        lua_newtable(L);  // font table

        lua_pushstring(L, fontname.c_str());  // value
        lua_setfield(L, -2, "name");          // insert

        lua_pushnumber(L, size);      // value
        lua_setfield(L, -2, "size");  // insert

        lua_setfield(L, -2, "font");  // insert font table

        lua_pushinteger(L, int(uint32_t(color)));  // value
        lua_setfield(L, -2, "color");              // insert
        // results in {font={name="fontname", size=0}, color=0x0}
    } else if (strcmp(mode, "selection") == 0) {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (!sel) {
            return luaL_error(L, "There is no selection! ");
        }
        auto rect = sel->getRect();

        lua_pushnumber(L, sel->getRotation());
        lua_setfield(L, -2, "rotation");

        lua_pushboolean(L, sel->isRotationSupported());
        lua_setfield(L, -2, "isRotationSupported");

        lua_newtable(L);  // create originalBounds table
        pushRectangleHelper(L, sel->getOriginalBounds());
        lua_setfield(L, -2, "originalBounds");  // add originalBounds table to return table

        lua_newtable(L);  // create snappedBounds table
        pushRectangleHelper(L, sel->getSnappedBounds());
        lua_setfield(L, -2, "snappedBounds");  // add snappedBounds table to return table

        lua_newtable(L);  // create boundingBox table
        pushRectangleHelper(L, rect);
        lua_setfield(L, -2, "boundingBox");  // add boundingBox table to return table
    }
    return 1;
}

/*
 * Returns a table encoding the document structure in a Lua table of the shape
 * {
 *   "pages" = {
 *     {
 *       "pageWidth" = number,
 *       "pageHeight" = number,
 *       "isAnnotated" = bool,
 *       "pageTypeFormat" = string,
 *       "pageTypeConfig" = string,
 *       "backgroundColor" = integer,
 *       "pdfBackgroundPageNo" = integer (0, if there is no pdf background page),
 *       "layers" = {
 *         [0] = {
 *           "isVisible" = bool
 *         },
 *         [1] = {
 *           "isVisible" = bool,
 *           "isAnnotated" = bool
 *         },
 *         ...
 *       },
 *       "currentLayer" = integer
 *     },
 *     ...
 *   }
 *   "currentPage" = integer,
 *   "pdfBackgroundFilename" = string (empty if there is none)
 *   "xoppFilename" = string (empty if there is none)
 * }
 *
 * Example: local docStructure = app.getDocumentStructure()
 */
static int applib_getDocumentStructure(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    Document* doc = control->getDocument();

    lua_newtable(L);

    lua_pushliteral(L, "pages");
    lua_newtable(L);  // beginning of pages table

    // stack now has following:
    //   -2 = table to be returned
    //   -1 = pages table/array

    // add pages
    for (size_t p = 1; p <= doc->getPageCount(); ++p) {
        auto page = doc->getPage(p - 1);
        lua_pushinteger(L, p);  // key of the page
        lua_newtable(L);        // beginning of table for page p

        lua_pushnumber(L, page->getWidth());  // value
        lua_setfield(L, -2, "pageWidth");     // insert

        lua_pushnumber(L, page->getHeight());  // value
        lua_setfield(L, -2, "pageHeight");     // insert

        lua_pushboolean(L, page->isAnnotated());  // value
        lua_setfield(L, -2, "isAnnotated");       // insert

        PageType pt = page->getBackgroundType();
        std::string pageTypeFormat = PageTypeHandler::getStringForPageTypeFormat(pt.format);
        lua_pushstring(L, pageTypeFormat.c_str());  // value
        lua_setfield(L, -2, "pageTypeFormat");      // insert

        lua_pushstring(L, pt.config.c_str());   // value
        lua_setfield(L, -2, "pageTypeConfig");  // insert

        lua_pushinteger(L, int(uint32_t(page->getBackgroundColor())));  // value
        lua_setfield(L, -2, "backgroundColor");                         // insert

        lua_pushinteger(L, page->getPdfPageNr() + 1);  // value
        lua_setfield(L, -2, "pdfBackgroundPageNo");    // insert

        lua_newtable(L);  // beginning of layers table

        // add background layer
        lua_pushinteger(L, 0);  // key of the layer
        lua_newtable(L);        // beginning of table for background layer

        lua_pushboolean(L, page->isLayerVisible(0U));  // value
        lua_setfield(L, -2, "isVisible");              // insert

        lua_pushstring(L, page->getBackgroundName().c_str());  // value
        lua_setfield(L, -2, "name");                           // insert

        lua_settable(L, -3);  // end of table for background layer

        // add (non-background) layers
        int currLayer = 0;

        for (auto l: *page->getLayers()) {
            lua_pushinteger(L, ++currLayer);  // key of the layer
            lua_newtable(L);                  // beginning of table for layer l

            lua_pushstring(L, l->getName().c_str());  // value
            lua_setfield(L, -2, "name");              // insert

            lua_pushboolean(L, l->isVisible());  // value
            lua_setfield(L, -2, "isVisible");    // insert

            lua_pushboolean(L, l->isAnnotated());  // value
            lua_setfield(L, -2, "isAnnotated");    // insert

            lua_settable(L, -3);  // end of table for layer l
        }
        lua_setfield(L, -2, "layers");  // end of layers table

        lua_pushinteger(L, page->getSelectedLayerId());  // value
        lua_setfield(L, -2, "currentLayer");             // insert

        lua_settable(L, -3);  // end of table for page p
    }
    lua_settable(L, -3);  // end of pages table

    lua_pushinteger(L, control->getCurrentPageNo() + 1);  // value
    lua_setfield(L, -2, "currentPage");                   // insert

    lua_pushstring(L, doc->getPdfFilepath().string().c_str());  // value
    lua_setfield(L, -2, "pdfBackgroundFilename");               // insert

    lua_pushstring(L, doc->getFilepath().string().c_str());  // value
    lua_setfield(L, -2, "xoppFilename");                     // insert

    return 1;
}

/**
 * Scrolls to the page specified relatively or absolutely (by default)
 * The page number is clamped to the range between the first and last page
 *
 * Example 1: app.scrollToPage(1, true)
 * scrolls to the next page (relative mode)
 *
 * Example 2: app.scrollToPage(10)
 * scrolls to page 10 (absolute mode)
 **/
static int applib_scrollToPage(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();

    int val = luaL_checkinteger(L, 1);
    bool relative = false;
    if (lua_isboolean(L, 2)) {
        relative = lua_toboolean(L, 2);
    }
    int page = (relative) ? control->getCurrentPageNo() + val : val - 1;

    const int first = 0;
    const int last = static_cast<int>(control->getDocument()->getPageCount()) - 1;
    control->getScrollHandler()->scrollToPage(std::clamp(page, first, last));

    return 0;
}

/**
 * Scrolls to the position on the selected page specified relatively (by default) or absolutely
 *
 * Example 1: app.scrollToPos(20,10)
 * scrolls 20pt right and 10pt down (relative mode)
 *
 * Example 2: app.scrollToPos(200, 50, false)
 * scrolls to page position 200pt right and 50pt down from the left page corner  (absolute mode)
 **/
static int applib_scrollToPos(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    Layout* layout = control->getWindow()->getLayout();

    double dx = luaL_checknumber(L, 1);
    double dy = luaL_checknumber(L, 2);
    bool relative = true;
    if (lua_isboolean(L, 3)) {
        relative = lua_toboolean(L, 3);
    }

    if (relative) {
        layout->scrollRelative(dx, dy);
    } else {
        layout->scrollAbs(dx, dy);
    }

    return 0;
}

/**
 * Sets the current page as indicated (without scrolling)
 * The page number passed is clamped to the range between first page and last page
 *
 * Example: app.setCurrentPage(1)
 * makes the first page the new current page
 **/
static int applib_setCurrentPage(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    size_t pageId = luaL_checkinteger(L, 1);
    const size_t first = 1;
    const size_t last = control->getDocument()->getPageCount();
    control->firePageSelected(std::clamp(pageId, first, last) - 1);

    return 0;
}

/**
 * Sets the width and height of the current page in pt = 1/72 inch either relatively or absolutely (by default)
 *
 * Example 1: app.setPageSize(595.275591, 841.889764)
 * makes the current page have standard (A4 paper) width and height (absolute mode)
 *
 * Example 2: app.setPageSize(0, 14.17*6, true)
 * adds 14.17*6 pt = 3cm to the height of the page (relative mode)
 **/
static int applib_setPageSize(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    Document* doc = control->getDocument();
    PageRef const& page = control->getCurrentPage();

    if (!page) {
        return luaL_error(L, "No page!");
    }

    double width = luaL_checknumber(L, 1);
    double height = luaL_checknumber(L, 2);

    bool relative = false;
    if (lua_isboolean(L, 3)) {
        relative = lua_toboolean(L, 3);
    }

    if (relative) {
        width += page->getWidth();
        height += page->getHeight();
    }

    if (width > 0 && height > 0) {
        doc->lock();
        Document::setPageSize(page, width, height);
        doc->unlock();
    }

    size_t pageNo = doc->indexOf(page);
    if (pageNo != npos && pageNo < doc->getPageCount()) {
        control->firePageSizeChanged(pageNo);
    }

    return 0;
}

/**
 * Sets the current layer of the current page as indicated and updates visibility if specified (by default it does not)
 * Displays an error message, if the selected layer does not exist
 *
 * Example 1: app.setCurrentLayer(2, true)
 * makes the second (non-background) layer the current layer and makes layers 1, 2 and the background layer visible, the
 *others hidden
 *
 * Example 2: app.setCurrentLayer(2, false)
 * makes the second (non-background) layer the current layer and does not change visibility
 **/
static int applib_setCurrentLayer(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    PageRef const& page = control->getCurrentPage();

    if (!page) {
        return luaL_error(L, "No page!");
    }

    size_t layerCount = page->getLayerCount();
    size_t layerId = luaL_checkinteger(L, 1);

    if (layerId > layerCount) {
        return luaL_error(L, "No layer with layer ID %d", layerId);
    }

    bool update = false;
    if (lua_isboolean(L, 2)) {
        update = lua_toboolean(L, 2);
    }

    control->getLayerController()->switchToLay(layerId, update);

    return 0;
}

/*
 * Sets the visibility of the current layer
 *
 * Example: app.setLayerVisibility(true)
 * makes the current layer visible
 */
static int applib_setLayerVisibility(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();

    bool enabled = true;
    if (lua_isboolean(L, 1)) {
        enabled = lua_toboolean(L, 1);
    }

    auto layerId = control->getCurrentPage()->getSelectedLayerId();
    control->getLayerController()->setLayerVisible(layerId, enabled);
    return 0;
}

/**
 * Sets currently selected layer's name.
 *
 * Example: app.setCurrentLayerName("Custom name 1")
 * Changes current layer name to "Custom name 1"
 **/
static int applib_setCurrentLayerName(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();

    if (lua_isstring(L, 1)) {
        auto name = lua_tostring(L, 1);
        control->getLayerController()->setCurrentLayerName(name);
    }

    return 0;
}

/**
 * Sets background name.
 *
 * Example: app.setBackgroundName("Custom name 1")
 * Changes background name to "Custom name 1"
 **/
static int applib_setBackgroundName(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    PageRef const& page = control->getCurrentPage();

    if (!page) {
        return luaL_error(L, "No page!");
    }

    if (lua_isstring(L, 1)) {
        auto name = lua_tostring(L, 1);
        page->setBackgroundName(name);
    }

    return 0;
}


/**
 * Scales all text elements of the current layer by the given scale factor.
 * This means the font sizes get scaled, wheras the position of the left upper corner
 * of the bounding box remains unchanged
 *
 * Example: app.scaleTextElements(2.3)
 * scales all text elements on the current layer with factor 2.3
 **/
static int applib_scaleTextElements(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();

    double f = luaL_checknumber(L, 1);

    control->clearSelectionEndText();

    const std::vector<Element*>& elements = control->getCurrentPage()->getSelectedLayer()->getElements();

    for (Element* e: elements) {
        if (e->getType() == ELEMENT_TEXT) {
            Text* t = static_cast<Text*>(e);
            t->scale(t->getX(), t->getY(), f, f, 0.0, false);
        }
    }

    return 0;
}


/**
 * Gets the display DPI.
 * Example: app.getDisplayDpi()
 **/
static int applib_getDisplayDpi(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    int dpi = control->getSettings()->getDisplayDpi();
    lua_pushinteger(L, dpi);

    return 1;
}


/**
 * Exports the current document as a pdf or as a svg or png image

 * Example 1:
 * app.export({["outputFile"] = "Test.pdf", ["range"] = "2-5; 7", ["background"] = "none", ["progressiveMode"] = true})
 * uses progressiveMode, so for each page of the document, instead of rendering one PDF page, the page layers are
 * rendered one by one to produce as many pages as there are layers.
 *
 * Example 2:
 * app.export({["outputFile"] = "Test.svg", ["range"] = "3-", ["background"] = "unruled"})
 *
 * Example 3:
 * app.export({["outputFile"] = "Test.png", ["layerRange"] = "1-2", ["background"] = "all", ["pngWidth"] = 800})
 **/
static int applib_export(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();
    Document* doc = control->getDocument();

    // discard any extra arguments passed in
    lua_settop(L, 1);
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "outputFile");
    lua_getfield(L, 1, "range");
    lua_getfield(L, 1, "layerRange");
    lua_getfield(L, 1, "background");
    lua_getfield(L, 1, "progressiveMode");
    lua_getfield(L, 1, "pngDpi");
    lua_getfield(L, 1, "pngWidth");
    lua_getfield(L, 1, "dpiHeight");

    // stack now has following:
    //    1 = param table
    //   -8 = outputFile
    //   -7 = range
    //   -6 = layerRange
    //   -5 = background
    //   -4 = progressiveMode
    //   -3 = pngDpi
    //   -2 = pngWidth
    //   -1 = dpiHeight

    const char* outputFile = luaL_optstring(L, -8, nullptr);
    const char* range = luaL_optstring(L, -7, nullptr);
    const char* layerRange = luaL_optstring(L, -6, nullptr);
    const char* background = luaL_optstring(L, -5, "all");
    bool progressiveMode = lua_toboolean(L, -4);  // true unless nil or false
    int pngDpi = luaL_optinteger(L, -3, -1);
    int pngWidth = luaL_optinteger(L, -2, -1);
    int pngHeight = luaL_optinteger(L, -1, -1);

    ExportBackgroundType bgType = EXPORT_BACKGROUND_ALL;
    if (strcmp(background, "unruled") == 0) {
        bgType = EXPORT_BACKGROUND_UNRULED;
    } else if (strcmp(background, "none") == 0) {
        bgType = EXPORT_BACKGROUND_NONE;
    }

    if (outputFile == nullptr) {
        return luaL_error(L, "Missing output file!");
    }

    fs::path file = fs::path(outputFile);
    auto extension = file.extension();

    if (extension == ".pdf") {
        ExportHelper::exportPdf(doc, outputFile, range, layerRange, bgType, progressiveMode);
    } else if (extension == ".svg" || extension == ".png") {
        ExportHelper::exportImg(doc, outputFile, range, layerRange, pngDpi, pngWidth, pngHeight, bgType);
    }

    return 0;
}

/**
 * Opens a file and by default asks the user what to do with the old document.
 * Returns true when successful, false otherwise.
 *
 * Example 1: local success = app.openFile("home/username/bg.pdf")
 *            asks what to do with the old document and loads the file afterwards, scrolls to top
 *
 * Example 2: local sucess = app.openFile("home/username/paintings.xopp", 3, true)
 *            opens the file, scrolls to the 3rd page and does not ask to save the old document
 */

static int applib_openFile(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();

    const char* filename = luaL_checkstring(L, 1);

    int scrollToPage = 0;  // by default scroll to the same page like last time
    if (lua_isinteger(L, 2)) {
        scrollToPage = lua_tointeger(L, 2);
    }

    bool forceOpen = false;  // by default asks the user
    if (lua_isboolean(L, 3)) {
        forceOpen = lua_toboolean(L, 3);
    }

    const bool success = control->openFile(fs::path(filename), scrollToPage - 1, forceOpen);
    lua_pushboolean(L, success);
    return 1;
}

/*
 * Adds images from the provided paths or the provided image data on the current page on the current layer.
 *
 * Global parameters:
 *  - images table: array of image-parameter-tables
 *  - allowUndoRedoAction string: Decides how the change gets introduced into the undoRedo action list "individual",
 *    "grouped" or "none"
 *
 * Parameters per image:
 *  - path string: filepath to the image
 *    or
 * - data string: (file) content of the image (exactly one of path and data should be specified)
 *  - x number: x-Coordinate where to place the left upper corner of the image (default: 0)
 *  - y number: y-Coordinate where to place the left upper corner of the image (default: 0)
 *  scaling options:
 *  - maxWidth integer: sets the width of the image in pixels. If that is too large for the page the image gets scaled
 *    down keeping the aspect-ratio provided by maxWidth/maxHeight (default: -1)
 *  - maxHeight integer: sets the height of the image in pixels. If that is too large for the page the image gets scaled
 *    down keeping the aspect-ratio provided by maxWidth/maxHeight (default: -1)
 *  - aspectRatio boolean: should the image be scaled in the other dimension to preserve the aspect ratio? Is only set
 *    to false if the parameter is false, nil leads to the default value of true (default: true)
 *  - scale number: factor to apply to the both dimensions in the end after setting width/height (with/without
 *    keeping aspect ratio) (default: 1)
 *
 * Note about scaling:
 * If the maxHeight-, the maxWidth- as well as the aspectRatio-parameter are given, the image will fit into the
 * rectangle specified with maxHeight/maxWidth. To preserve the aspect ratio, one dimension will be smaller than
 * specified. Still, the scale parameter is applied after this width/height scaling and if after that the dimensions are
 * too large for the page, the image still gets scaled down afterwards.
 *
 * Returns as many values as images were passed. A nil value represents success, while
 * on error the value corresponding to the image will be a string with the error message.
 * If the parameters don't fit at all, a real lua error might be thrown immediately.
 *
 * Example 1: app.addImages{images={{path="/media/data/myImg.png", x=10, y=20, scale=2},
 *                                            {path="/media/data/myImg2.png", maxHeight=300, aspectRatio=true}},
 *                                    allowUndoRedoAction="grouped",
 *                                            }
 * Example 2: app.addImages{images={{data="<binary image data>", x=100, y=200, maxHeight=300, maxWidth=400}}}
 */
static int applib_addImages(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    Control* control = plugin->getControl();

    // Discard any extra arguments passed in
    lua_settop(L, 1);

    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "images");
    if (!lua_istable(L, 2)) {
        return luaL_error(L, "Missing image table!");
    }

    size_t cntParams = lua_rawlen(L, 2);

    std::vector<Element*> images{};
    for (int imgParam{1}; imgParam <= cntParams; imgParam++) {

        lua_pushnumber(L, imgParam);
        lua_gettable(L, 2);
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_getfield(L, -1, "path");
        lua_getfield(L, -2, "data");
        lua_getfield(L, -3, "x");
        lua_getfield(L, -4, "y");
        lua_getfield(L, -5, "maxWidth");
        lua_getfield(L, -6, "maxHeight");
        lua_getfield(L, -7, "scale");
        lua_getfield(L, -8, "aspectRatio");

        // stack now has the following:
        //    1 = global params table
        //   -9 = current img-params table
        //   -8 = filepath
        //   -7 = image data
        //   -6 = x coordinate
        //   -5 = y coordinate
        //   -4 = maxWidth (in pixel)
        //   -3 = maxHeight (in pixel)
        //   -2 = scale
        //   -1 = aspectRatio

        // fetch the parameters and check for validity. If parameter is invalid -> hard error
        double x = luaL_optnumber(L, -6, 0);
        double y = luaL_optnumber(L, -5, 0);

        int maxHeightParam = luaL_optinteger(L, -3, -1);
        if (maxHeightParam <= 0 && maxHeightParam != -1) {
            return luaL_error(L, "Invalid height given, must be positive integer or -1 to deactivate manual setting.");
        }

        int maxWidthParam = luaL_optinteger(L, -4, -1);
        if (maxWidthParam <= 0 && maxWidthParam != -1) {
            return luaL_error(L, "Invalid width given, must be positive integer or -1 to deactivate manual setting.");
        }

        double scale = luaL_optnumber(L, -2, 1);
        if (scale <= 0) {
            return luaL_error(L, "Invalid scale given, must be a positive number.");
        }

        bool aspectRatio{true};
        if (!lua_isnil(L, -1)) {
            // use the typical lua version of booleans (everything different than false (any set value) is true)
            aspectRatio = lua_toboolean(L, -1);
        }

        const char* path = luaL_optstring(L, -8, nullptr);
        size_t dataLen = 0;
        const char* data = luaL_optlstring(L, -7, nullptr, &dataLen);
        if (!path && !data) {
            return luaL_error(L, "no 'path' parameter and no image 'data' was provided.");
        }
        if (path && data) {
            return luaL_error(L,
                              "both 'path' parameter and image 'data' were provided. Only one should be specified. ");
        }

        Image* img;
        int width;
        int height;
        XojPageView* pv = control->getWindow()->getXournal()->getViewFor(control->getCurrentPageNo());
        ImageHandler imgHandler(control, pv);
        if (path) {
            xoj::util::GObjectSPtr<GFile> file(g_file_new_for_path(path), xoj::util::adopt);
            if (!g_file_query_exists(file.get(), NULL)) {
                lua_pop(L, 8);  // pop the params we fetched from the global param-table from the stack
                lua_pushfstring(L, "Error: file '%s' does not exist.", path);  // soft error
                continue;
            }

            std::tie(img, width, height) = imgHandler.createImageFromFile(file.get(), x, y);
            if (!img) {
                lua_pop(L, 8);  // pop the params we fetched from the global param-table from the stack
                lua_pushfstring(L, "Error: creating the image (%s) failed.", path);  // soft error
                continue;
            }
        } else {  // data was provided instead
            img = new Image();
            img->setImage(std::string(data, dataLen));
            img->getImage();  // render image first to get the proper width and height
            std::tie(width, height) = img->getImageSize();
            img->setX(x);
            img->setY(y);
        }

        // apply width/height parameter
        if (maxWidthParam != -1 && maxHeightParam != -1) {
            // both width and height are set
            if (aspectRatio) {
                double scale_y{static_cast<double>(maxHeightParam) / static_cast<double>(height)};
                double scale_x{static_cast<double>(maxWidthParam) / static_cast<double>(width)};
                double scale{std::min(scale_y, scale_x)};

                height = static_cast<int>(std::round(height * scale));
                width = static_cast<int>(std::round(width * scale));
            } else {
                width = maxWidthParam;
                height = maxHeightParam;
            }
        } else if (maxWidthParam != -1 && maxHeightParam == -1) {
            // maxHeight is set
            if (aspectRatio) {
                height = static_cast<int>(
                        std::round(static_cast<double>(height) / static_cast<double>(width) * maxWidthParam));
            }
            width = maxWidthParam;
        } else if (maxHeightParam != -1 && maxWidthParam == -1) {
            // maxWidth is set
            if (aspectRatio) {
                width = static_cast<int>(
                        std::round(static_cast<double>(width) / static_cast<double>(height) * maxHeightParam));
            }
            height = maxHeightParam;
        }

        // apply scale option
        width = static_cast<int>(std::round(width * scale));
        height = static_cast<int>(std::round(height * scale));

        // scale down keeping the current aspect ratio after the manual scaling to fit the image on the page
        // if the image already fits on the screen, no other scaling is applied here
        // already sets width/height in the image
        imgHandler.automaticScaling(img, x, y, width, height);

        // store the image to later build the undo/redo action chain
        images.push_back(img);

        lua_pop(L, 8);  // pop the params we fetched from the global param-table from the stack

        bool succ = imgHandler.addImageToDocument(img, false);
        if (!succ) {
            lua_pushfstring(L, "Error: Inserting the image (%s) failed.", path);  // soft error
        }

        lua_pushnil(L);
    }

    // Check how the user wants to handle undoing
    lua_getfield(L, 1, "allowUndoRedoAction");
    const char* allowUndoRedoAction = luaL_optstring(L, -1, "grouped");
    lua_pop(L, 1);
    handleUndoRedoActionHelper(L, control, allowUndoRedoAction, images);

    return static_cast<int>(cntParams);
}

/**
 * Puts a Lua Table of the Images (from the selection tool / selected layer) onto the stack.
 * Is inverse to app.addImages
 *
 * Required argument: type ("selection" or "layer")
 *
 * Example: local images = app.getImages("selection")
 *
 * return value:
 * {
 *     {
 *         ["x"] = number,
 *         ["y"] = number,
 *         ["width"] = number,    (width when inserted into Xournal++)
 *         ["height"] = number,   (height when inserted into Xournal++)
 *         ["data"] = string,
 *         ["format"] = string,
 *         ["imageWidth"] = integer,
 *         ["imageHeight"] = integer,
 *     },
 *     {
 *         ...
 *     },
 *     ...
 * }
 */

static int applib_getImages(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    std::string type = luaL_checkstring(L, 1);
    std::vector<Element*> elements = {};
    Control* control = plugin->getControl();

    // Discard any extra arguments passed in
    lua_settop(L, 1);

    if (type == "layer") {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (sel) {
            control->clearSelection();  // otherwise strokes in the selection won't be recognized
        }
        elements = control->getCurrentPage()->getSelectedLayer()->getElements();
    } else if (type == "selection") {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (sel) {
            elements = sel->getElements();
        } else {
            return luaL_error(L, "There is no selection! ");
        }
    } else {
        return luaL_error(L, "Unknown argument: %s", type.c_str());
    }

    lua_newtable(L);  // create table of all images
    int currImageNo = 0;

    for (Element* e: elements) {
        if (e->getType() == ELEMENT_IMAGE) {
            auto* im = static_cast<Image*>(e);
            lua_pushnumber(L, ++currImageNo);  // index for later (settable)
            lua_newtable(L);                   // create table for current image

            // "x": number
            lua_pushnumber(L, im->getX());
            lua_setfield(L, -2, "x");

            // "y": number
            lua_pushnumber(L, im->getY());
            lua_setfield(L, -2, "y");

            // "width": number
            lua_pushnumber(L, im->getElementWidth());
            lua_setfield(L, -2, "width");

            // "height": number
            lua_pushnumber(L, im->getElementHeight());
            lua_setfield(L, -2, "height");

            // data: string (can be optimized via lual_Buffer)
            lua_pushlstring(L, reinterpret_cast<const char*>(im->getRawData()), im->getRawDataLength());
            lua_setfield(L, -2, "data");

            // format: string
            lua_pushstring(L, gdk_pixbuf_format_get_name(im->getImageFormat()));
            lua_setfield(L, -2, "format");

            std::pair<int, int> imageSize = im->getImageSize();
            // image width: integer
            lua_pushinteger(L, imageSize.first);
            lua_setfield(L, -2, "imageWidth");
            // image height: integer
            lua_pushinteger(L, imageSize.second);
            lua_setfield(L, -2, "imageHeight");

            lua_settable(L, -3);  // add image to table
        }
    }
    return 1;
}


/*
 * The full Lua Plugin API.
 * See above for example usage of each function.
 */
static const luaL_Reg applib[] = {{"msgbox", applib_msgbox},  // Todo(gtk4) remove this deprecated function
                                  {"openDialog", applib_openDialog},
                                  {"glib_rename", applib_glib_rename},
                                  {"saveAs", applib_saveAs},
                                  {"registerUi", applib_registerUi},
                                  {"uiAction", applib_uiAction},
                                  {"uiActionSelected", applib_uiActionSelected},
                                  {"sidebarAction", applib_sidebarAction},
                                  {"layerAction", applib_layerAction},
                                  {"changeToolColor", applib_changeToolColor},
                                  {"changeCurrentPageBackground", applib_changeCurrentPageBackground},
                                  {"changeBackgroundPdfPageNr", applib_changeBackgroundPdfPageNr},
                                  {"getToolInfo", applib_getToolInfo},
                                  {"getSidebarPageNo", applib_getSidebarPageNo},
                                  {"setSidebarPageNo", applib_setSidebarPageNo},
                                  {"getDocumentStructure", applib_getDocumentStructure},
                                  {"scrollToPage", applib_scrollToPage},
                                  {"scrollToPos", applib_scrollToPos},
                                  {"setCurrentPage", applib_setCurrentPage},
                                  {"setPageSize", applib_setPageSize},
                                  {"setCurrentLayer", applib_setCurrentLayer},
                                  {"setLayerVisibility", applib_setLayerVisibility},
                                  {"setCurrentLayerName", applib_setCurrentLayerName},
                                  {"setBackgroundName", applib_setBackgroundName},
                                  {"scaleTextElements", applib_scaleTextElements},
                                  {"getDisplayDpi", applib_getDisplayDpi},
                                  {"export", applib_export},
                                  {"addStrokes", applib_addStrokes},
                                  {"addSplines", applib_addSplines},
                                  {"addImages", applib_addImages},
                                  {"addTexts", applib_addTexts},
                                  {"getFilePath", applib_getFilePath},
                                  {"refreshPage", applib_refreshPage},
                                  {"getStrokes", applib_getStrokes},
                                  {"getImages", applib_getImages},
                                  {"getTexts", applib_getTexts},
                                  {"openFile", applib_openFile},
                                  // Placeholder
                                  //	{"MSG_BT_OK", nullptr},

                                  {nullptr, nullptr}};

/**
 * Open application Library
 */
inline int luaopen_app(lua_State* L) {
    luaL_newlib(L, applib);
    //	lua_pushnumber(L, MSG_BT_OK);
    //	lua_setfield(L, -2, "MSG_BT_OK");
    return 1;
}
