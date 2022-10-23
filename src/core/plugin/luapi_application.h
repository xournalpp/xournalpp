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
#include <cstring>
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
    GFile* to = g_file_new_for_path(lua_tostring(L, -1));
    GFile* from = g_file_new_for_path(lua_tostring(L, -2));

    g_file_move(from, to, G_FILE_COPY_OVERWRITE, nullptr, nullptr, nullptr, &err);
    g_object_unref(from);
    g_object_unref(to);
    if (err) {
        lua_pushnil(L);
        lua_pushfstring(L, "%s (error code: %d)", err->message, err->code);
        g_error_free(err);
        return 2;
    } else {
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
    GtkFileChooserNative* native;
    gint res;
    int args_returned = 0;  // change to 1 if user chooses file

    const char* filename = luaL_checkstring(L, -1);

    // Create a 'Save As' native dialog
    native = gtk_file_chooser_native_new(_("Save file"), nullptr, GTK_FILE_CHOOSER_ACTION_SAVE, nullptr, nullptr);

    // If user tries to overwrite a file, ask if it's OK
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(native), TRUE);
    // Offer a suggestion for the filename if filename absent
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(native),
                                      filename ? filename : (std::string{_("Untitled")}).c_str());

    // Wait until user responds to dialog
    res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));

    // Return the filename chosen to lua
    if (res == GTK_RESPONSE_ACCEPT) {
        char* filename = static_cast<char*>(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native)));

        lua_pushlstring(L, filename, strlen(filename));
        g_free(static_cast<gchar*>(filename));
        args_returned = 1;
    }

    // Destroy the dialog and free memory
    g_object_unref(native);

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
    GtkFileChooserNative* native =
            gtk_file_chooser_native_new(_("Open file"), nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, nullptr, nullptr);
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
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(native), filterSupported);
    }

    // Wait until user responds to dialog
    res = gtk_native_dialog_run(GTK_NATIVE_DIALOG(native));
    // Return the filename chosen to lua
    if (res == GTK_RESPONSE_ACCEPT) {
        filename = static_cast<char*>(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native)));
        lua_pushlstring(L, filename, strlen(filename));
        g_free(static_cast<gchar*>(filename));
        args_returned = 1;
    }
    // Destroy the dialog and free memory
    g_object_unref(native);
    return args_returned;
}

/**
 * Example: local result = app.msgbox("Test123", {[1] = "Yes", [2] = "No"})
 * Pops up a message box with two buttons "Yes" and "No" and returns 1 for yes, 2 for no
 */
static int applib_msgbox(lua_State* L) {
    const char* msg = luaL_checkstring(L, 1);

    // discard any extra arguments passed in
    lua_settop(L, 2);
    luaL_checktype(L, 2, LUA_TTABLE);

    lua_pushnil(L);

    std::map<int, std::string> button;

    while (lua_next(L, 2) != 0) {
        int index = lua_tointeger(L, -2);
        const char* buttonText = luaL_checkstring(L, -1);
        lua_pop(L, 1);

        button.insert(button.begin(), std::pair<int, std::string>(index, buttonText));
    }

    Plugin* plugin = Plugin::getPluginFromLua(L);

    int result = XojMsgBox::showPluginMessage(plugin->getName(), msg, button);
    lua_pushinteger(L, result);
    return 1;
}


/**
 * Allow to register menupoints, this needs to be called from initUi
 *
 * Example: app.registerUi({["menu"] = "HelloWorld", callback="printMessage", mode=1, accelerator="<Control>a"})
 * registers a menupoint with name "HelloWorld" executing a function named "printMessage", in mode 1,
 * which can be triggered via the "<Control>a" keyboard accelerator
 *
 * The mode and accelerator are optional. When specifying the mode, the callback function should have one parameter
   that receives the mode. This is useful for callback functions that are shared among multiple menu entries.
 */
static int applib_registerUi(lua_State* L) {
    Plugin* plugin = Plugin::getPluginFromLua(L);
    if (!plugin->isInInitUi()) {
        luaL_error(L, "registerUi needs to be called within initUi()");
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
    // stack now has following:
    //    1 = {"menu"="MenuName", callback="functionName", mode=1, accelerator="<Control>a"}
    //   -4 = "<Control>a"
    //   -3 = "MenuName"
    //   -2 = "functionName"
    //   -1 = mode

    const char* accelerator = luaL_optstring(L, -4, nullptr);
    const char* menu = luaL_optstring(L, -3, nullptr);
    const char* callback = luaL_optstring(L, -2, nullptr);
    const long mode = luaL_optinteger(L, -1, LONG_MAX);
    if (callback == nullptr) {
        luaL_error(L, "Missing callback function!");
    }
    if (menu == nullptr) {
        menu = "";
    }
    if (accelerator == nullptr) {
        accelerator = "";
    }

    int toolbarId = -1;

    int menuId = plugin->registerMenu(menu, callback, mode, accelerator);

    // Make sure to remove all vars which are put to the stack before!
    lua_pop(L, 4);

    // Add return value to the Stack
    lua_createtable(L, 0, 2);

    lua_pushstring(L, "menuId");
    lua_pushinteger(L, menuId);
    lua_settable(L, -3); /* 3rd element from the stack top */

    lua_pushstring(L, "toolbarId");
    lua_pushinteger(L, toolbarId);
    lua_settable(L, -3);

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
        luaL_error(L, "Missing action!");
    }

    ActionType action = ActionType_fromString(actionStr);
    GtkToolButton* toolbutton = nullptr;

    Control* ctrl = plugin->getControl();
    ctrl->actionPerformed(action, group, toolbutton, enabled);

    // Make sure to remove all vars which are put to the stack before!
    lua_pop(L, 3);

    return 1;
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

    return 1;
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
        luaL_error(L, "Missing action!");
    }
    auto pos = actionMap.find(actionStr);
    if (pos == actionMap.end()) {
        luaL_error(L, "Unkonwn action: %s", actionStr);
    }
    Plugin* plugin = Plugin::getPluginFromLua(L);
    SidebarToolbar* toolbar = plugin->getControl()->getSidebar()->getToolbar();
    toolbar->runAction(pos->second);

    return 1;
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

    return 1;
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
        luaL_error(L, "Missing spline table!");

    size_t numSplines = lua_rawlen(L, -1);
    for (size_t a = 1; a <= numSplines; a++) {
        std::vector<double> coordStream;
        Stroke* stroke = new Stroke();
        // Get coordinates
        lua_pushnumber(L, a);
        lua_gettable(L, -2);
        lua_getfield(L, -1, "coordinates");
        if (!lua_istable(L, -1))
            luaL_error(L, "Missing coordinate table!");
        size_t numCoords = lua_rawlen(L, -1);
        for (size_t b = 1; b <= numCoords; b++) {
            lua_pushnumber(L, b);
            lua_gettable(L, -2);
            double point = lua_tonumber(L, -1);
            coordStream.push_back(point);  // Each segment is going to have multiples of 8 points.
            lua_pop(L, 1);
        }
        // pop value + copy of key, leaving original key
        lua_pop(L, 1);
        // Handle those points
        // Check if the list is divisible by 8.
        if (coordStream.size() % 8 != 0)
            luaL_error(L, "Point table incomplete!");

        // Now take that gigantic list of splines and create SplineSegments out of them.
        long unsigned int i = 0;
        while (i < coordStream.size()) {
            // start, ctrl1, ctrl2, end
            Point start = Point(coordStream.at(i), coordStream.at(i + 1), Point::NO_PRESSURE);
            Point ctrl1 = Point(coordStream.at(i + 2), coordStream.at(i + 3), Point::NO_PRESSURE);
            Point ctrl2 = Point(coordStream.at(i + 4), coordStream.at(i + 5), Point::NO_PRESSURE);
            Point end = Point(coordStream.at(i + 6), coordStream.at(i + 7), Point::NO_PRESSURE);
            i += 8;
            SplineSegment segment = SplineSegment(start, ctrl1, ctrl2, end);
            std::list<Point> raster = segment.toPointSequence();
            for (Point point: raster) stroke->addPoint(point);
            // TODO: (willnilges) Is there a way we can get Pressure with Splines?
        }

        if (stroke->getPointCount() >= 2) {
            // Finish building the Stroke and apply it to the layer.
            addStrokeHelper(L, stroke);
            strokes.push_back(stroke);
        } else
            g_warning("Stroke shorter than two points. Discarding. (Has %d)", stroke->getPointCount());
        // Onto the next stroke
        lua_pop(L, 1);
    }

    lua_pop(L, 1);  // Stack is now the same as it was on entry to this function

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
        g_warning("Unrecognized undo/redo option: %s", allowUndoRedoAction);
    lua_pop(L, 1);
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
        luaL_error(L, "Missing stroke table!");
    size_t numStrokes = lua_rawlen(L, -1);
    for (size_t a = 1; a <= numStrokes; a++) {
        std::vector<double> xStream;
        std::vector<double> yStream;
        std::vector<double> pressureStream;
        Stroke* stroke = new Stroke();

        // Fetch table of X values from the Lua stack
        lua_pushnumber(L, a);
        lua_gettable(L, -2);

        lua_getfield(L, -1, "x");
        if (!lua_istable(L, -1))
            luaL_error(L, "Missing X-Coordinate table!");
        size_t xPoints = lua_rawlen(L, -1);
        for (size_t b = 1; b <= xPoints; b++) {
            lua_pushnumber(L, b);
            lua_gettable(L, -2);
            double value = lua_tonumber(L, -1);
            xStream.push_back(value);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // Fetch table of Y values form the Lua stack
        lua_getfield(L, -1, "y");
        if (!lua_istable(L, -1))
            luaL_error(L, "Missing Y-Coordinate table!");
        size_t yPoints = lua_rawlen(L, -1);
        for (size_t b = 1; b <= yPoints; b++) {
            lua_pushnumber(L, b);
            lua_gettable(L, -2);
            double value = lua_tonumber(L, -1);
            yStream.push_back(value);
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // Fetch table of pressure values from the Lua stack
        lua_getfield(L, -1, "pressure");
        if (lua_istable(L, -1)) {
            size_t pressurePoints = lua_rawlen(L, -1);
            for (size_t b = 1; b <= pressurePoints; b++) {
                lua_pushnumber(L, b);
                lua_gettable(L, -2);
                double value = lua_tonumber(L, -1);
                pressureStream.push_back(value);
                lua_pop(L, 1);
            }
        } else
            g_warning("Missing pressure table. Assuming NO_PRESSURE.");

        lua_pop(L, 1);

        // Handle those points
        // Make sure all vectors are the same length.
        if (xStream.size() != yStream.size()) {
            luaL_error(L, "X and Y vectors are not equal length!");
        }
        if (xStream.size() != pressureStream.size() && pressureStream.size() > 0)
            luaL_error(L, "Pressure vector is not equal length!");

        // Check and make sure there's enough points (need at least 2)
        if (xStream.size() < 2) {
            g_warning("Stroke shorter than two points. Discarding. (Has %ld/2)", xStream.size());
            return 1;
        }
        // Add points to the stroke. Include pressure, if it exists.
        if (pressureStream.size() > 0) {
            for (long unsigned int i = 0; i < xStream.size(); i++) {
                Point myPoint = Point(xStream.at(i), yStream.at(i), pressureStream.at(i));
                stroke->addPoint(myPoint);
            }
        } else {
            for (long unsigned int i = 0; i < xStream.size(); i++) {
                Point myPoint = Point(xStream.at(i), yStream.at(i), Point::NO_PRESSURE);
                stroke->addPoint(myPoint);
            }
        }

        // Finish building the Stroke and apply it to the layer.
        addStrokeHelper(L, stroke);
        strokes.push_back(stroke);
        // Onto the next stroke
        lua_pop(L, 1);
    }

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
        g_warning("Unrecognized undo/redo option: %s", allowUndoRedoAction);

    lua_pop(L, 1);  // Stack is now the same as it was on entry to this function

    return 0;
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

    if (type == "Layer") {
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
            g_warning("There is no selection! ");
            return 0;
        }
    } else {
        g_warning("Unknown argument: %s", type.c_str());
        return 0;
    }

    lua_newtable(L);  // create table of the elements
    int currStrokeNo = 0;
    int currPointNo = 0;

    for (Element* e: elements) {
        if (e->getType() == ELEMENT_STROKE) {
            auto* s = static_cast<Stroke*>(e);
            lua_pushnumber(L, ++currStrokeNo);  // index for later (settable)
            lua_newtable(L);                    // create stroke table

            lua_newtable(L);  // create table of x-coordinates
            for (auto p: s->getPointVector()) {
                lua_pushnumber(L, ++currPointNo);
                lua_pushnumber(L, p.x);
                lua_settable(L, -3);  // pops key and value from stack
            }
            lua_setfield(L, -2, "x");  // add x-coordinates to stroke
            currPointNo = 0;

            lua_newtable(L);  // create table for y-coordinates
            for (auto p: s->getPointVector()) {
                lua_pushnumber(L, ++currPointNo);
                lua_pushnumber(L, p.y);
                lua_settable(L, -3);
            }
            lua_setfield(L, -2, "y");  // add y-coordinates to stroke
            currPointNo = 0;

            if (s->hasPressure()) {
                lua_newtable(L);  // create table for pressures
                for (auto p: s->getPointVector()) {
                    lua_pushnumber(L, ++currPointNo);
                    lua_pushnumber(L, p.z);
                    lua_settable(L, -3);
                }
                lua_setfield(L, -2, "pressure");  // add pressures to stroke
                currPointNo = 0;
            }

            StrokeTool tool = s->getToolType();
            if (tool == StrokeTool::PEN) {
                lua_pushstring(L, "pen");
            } else if (tool == StrokeTool::HIGHLIGHTER) {
                lua_pushstring(L, "highlighter");
            } else {
                g_warning("Unknown STROKE_TOOL. ");
                return 0;
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

            lua_settable(L, -3);  // add stroke to elements
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
        g_warning("Called applib_refreshPage, but there is no current page.");
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

    return 1;
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
        g_warning(""
                  "selection"
                  " key should be a boolean value (or nil)");
    }

    ToolType toolType = toolHandler->getToolType();
    const char* toolStr = luaL_optstring(L, -2, nullptr);
    if (toolStr != nullptr) {
        toolType = toolTypeFromString(StringUtils::toLowerCase(toolStr));
    }

    if (toolType == TOOL_NONE) {
        g_warning("tool \"%s\" is not valid or no tool has been selected", toolTypeToString(toolType).c_str());
        lua_pop(L, 3);
        return 0;
    }

    uint32_t color = 0x000000;
    if (lua_isinteger(L, -1)) {
        color = as_unsigned(lua_tointeger(L, -1));
        if (color > 0xffffff) {
            g_warning("Color 0x%x is no valid RGB color. ", color);
            return 0;
        }
    } else if (!lua_isnil(L, -1)) {
        g_warning(" "
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
        g_warning("tool \"%s\" has no color capability", toolTypeToString(toolType).c_str());
    }

    // Make sure to remove all vars which are put to the stack before!
    lua_pop(L, 3);

    return 1;
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
        luaL_error(L, "No page!");
    }

    size_t selected = nr - 1;
    if (relative) {
        bool isPdf = page->getBackgroundType().isPdfPage();
        if (isPdf) {
            selected = page->getPdfPageNr() + nr;
        } else {
            luaL_error(L, "Current page has no pdf background, cannot use relative mode!");
        }
    }
    if (selected < doc->getPdfPageCount()) {
        // no need to set a type, if we set the page number the type is also set
        page->setBackgroundPdfPageNr(selected);

        XojPdfPageSPtr p = doc->getPdfPage(selected);
        page->setSize(p->getWidth(), p->getHeight());
    } else {
        luaL_error(L, "Pdf page number %d does not exist!", selected + 1);
    }

    return 1;
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
 * Example 3: local color = app.getToollInfo("text")["color"]
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

    const char* mode = luaL_checkstring(L, -1);
    lua_newtable(L);

    if (strcmp(mode, "active") == 0) {
        std::string toolType = toolTypeToString(toolHandler->getToolType());

        std::string toolSize = toolSizeToString(toolHandler->getSize());
        double thickness = toolHandler->getThickness();

        Color color = toolHandler->getColor();
        int fillOpacity = toolHandler->getFill();
        std::string drawingType = drawingTypeToString(toolHandler->getDrawingType());
        std::string lineStyle = StrokeStyle::formatStyle(toolHandler->getLineStyle());


        lua_pushliteral(L, "type");
        lua_pushstring(L, toolType.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "size");
        lua_newtable(L);  // beginning of "size" table

        lua_pushliteral(L, "name");
        lua_pushstring(L, toolSize.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "value");
        lua_pushnumber(L, thickness);
        lua_settable(L, -3);

        lua_settable(L, -3);  // end of "size" table

        lua_pushliteral(L, "color");
        lua_pushinteger(L, int(uint32_t(color)));
        lua_settable(L, -3);

        lua_pushliteral(L, "fillOpacity");
        lua_pushinteger(L, fillOpacity);
        lua_settable(L, -3);

        lua_pushliteral(L, "drawingType");
        lua_pushstring(L, drawingType.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "lineStyle");
        lua_pushstring(L, lineStyle.c_str());
        lua_settable(L, -3);
    } else if (strcmp(mode, "pen") == 0) {
        std::string size = toolSizeToString(toolHandler->getPenSize());
        double thickness = toolHandler->getToolThickness(TOOL_PEN)[toolSizeFromString(size)];

        int fillOpacity = toolHandler->getPenFill();
        bool filled = toolHandler->getPenFillEnabled();

        Tool& tool = toolHandler->getTool(TOOL_PEN);
        Color color = tool.getColor();
        std::string drawingType = drawingTypeToString(tool.getDrawingType());
        std::string lineStyle = StrokeStyle::formatStyle(tool.getLineStyle());

        lua_pushliteral(L, "size");
        lua_newtable(L);  // beginning of "size" table

        lua_pushliteral(L, "name");
        lua_pushstring(L, size.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "value");
        lua_pushnumber(L, thickness);
        lua_settable(L, -3);

        lua_settable(L, -3);  // end of "size" table

        lua_pushliteral(L, "color");
        lua_pushinteger(L, int(uint32_t(color)));
        lua_settable(L, -3);

        lua_pushliteral(L, "drawingType");
        lua_pushstring(L, drawingType.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "lineStyle");
        lua_pushstring(L, lineStyle.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "filled");
        lua_pushboolean(L, filled);
        lua_settable(L, -3);

        lua_pushliteral(L, "fillOpacity");
        lua_pushinteger(L, fillOpacity);
        lua_settable(L, -3);
    } else if (strcmp(mode, "highlighter") == 0) {
        std::string size = toolSizeToString(toolHandler->getHighlighterSize());
        double thickness = toolHandler->getToolThickness(TOOL_HIGHLIGHTER)[toolSizeFromString(size)];

        int fillOpacity = toolHandler->getHighlighterFill();
        bool filled = toolHandler->getHighlighterFillEnabled();

        Tool& tool = toolHandler->getTool(TOOL_HIGHLIGHTER);
        Color color = tool.getColor();
        std::string drawingType = drawingTypeToString(tool.getDrawingType());

        lua_pushliteral(L, "size");
        lua_newtable(L);  // beginning of "size" table

        lua_pushliteral(L, "name");
        lua_pushstring(L, size.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "value");
        lua_pushnumber(L, thickness);
        lua_settable(L, -3);

        lua_settable(L, -3);  // end of "size" table

        lua_pushliteral(L, "color");
        lua_pushinteger(L, int(uint32_t(color)));
        lua_settable(L, -3);

        lua_pushliteral(L, "drawingType");
        lua_pushstring(L, drawingType.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "filled");
        lua_pushboolean(L, filled);
        lua_settable(L, -3);

        lua_pushliteral(L, "fillOpacity");
        lua_pushinteger(L, fillOpacity);
        lua_settable(L, -3);
    } else if (strcmp(mode, "eraser") == 0) {
        std::string type = eraserTypeToString(toolHandler->getEraserType());

        std::string size = toolSizeToString(toolHandler->getEraserSize());
        double thickness = toolHandler->getToolThickness(ToolType::TOOL_ERASER)[toolSizeFromString(size)];

        lua_pushliteral(L, "type");
        lua_pushstring(L, type.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "size");
        lua_newtable(L);  // beginning of "size" table

        lua_pushliteral(L, "name");
        lua_pushstring(L, size.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "value");
        lua_pushnumber(L, thickness);
        lua_settable(L, -3);

        lua_settable(L, -3);  // end of "size" table
    } else if (strcmp(mode, "text") == 0) {
        Settings* settings = control->getSettings();
        XojFont& font = settings->getFont();
        std::string fontname = font.getName();
        double size = font.getSize();

        Tool& tool = toolHandler->getTool(TOOL_TEXT);
        Color color = tool.getColor();

        lua_newtable(L);

        lua_pushliteral(L, "font");
        lua_newtable(L);

        lua_pushliteral(L, "name");
        lua_pushstring(L, fontname.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "size");
        lua_pushnumber(L, size);
        lua_settable(L, -3);

        lua_settable(L, -3);

        lua_pushliteral(L, "color");
        lua_pushinteger(L, int(uint32_t(color)));
        lua_settable(L, -3);
    } else if (strcmp(mode, "selection") == 0) {
        auto sel = control->getWindow()->getXournal()->getSelection();
        if (!sel) {
            g_warning("There is no selection! ");
            return 0;
        }
        auto rect = sel->getRect();

        lua_newtable(L);  // create return table
                          //
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
 *       "isAnnoated" = bool,
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

    // add pages
    for (size_t p = 1; p <= doc->getPageCount(); ++p) {
        auto page = doc->getPage(p - 1);
        lua_pushinteger(L, p);
        lua_newtable(L);  // beginning of table for page p

        lua_pushliteral(L, "pageWidth");
        lua_pushnumber(L, page->getWidth());
        lua_settable(L, -3);

        lua_pushliteral(L, "pageHeight");
        lua_pushnumber(L, page->getHeight());
        lua_settable(L, -3);

        lua_pushliteral(L, "isAnnotated");
        lua_pushboolean(L, page->isAnnotated());
        lua_settable(L, -3);

        lua_pushliteral(L, "pageTypeFormat");
        PageType pt = page->getBackgroundType();
        std::string pageTypeFormat = PageTypeHandler::getStringForPageTypeFormat(pt.format);
        lua_pushstring(L, pageTypeFormat.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "pageTypeConfig");
        lua_pushstring(L, pt.config.c_str());
        lua_settable(L, -3);

        lua_pushliteral(L, "backgroundColor");
        lua_pushinteger(L, int(uint32_t(page->getBackgroundColor())));
        lua_settable(L, -3);

        lua_pushliteral(L, "pdfBackgroundPageNo");
        lua_pushinteger(L, page->getPdfPageNr() + 1);
        lua_settable(L, -3);

        lua_pushstring(L, "layers");
        lua_newtable(L);  // beginning of layers table

        // add background layer
        lua_pushinteger(L, 0);
        lua_newtable(L);  // beginning of table for background layer

        lua_pushliteral(L, "isVisible");
        lua_pushboolean(L, page->isLayerVisible(0U));
        lua_settable(L, -3);

        lua_pushliteral(L, "name");
        lua_pushstring(L, page->getBackgroundName().c_str());
        lua_settable(L, -3);

        lua_settable(L, -3);  // end of table for background layer

        // add (non-background) layers
        int currLayer = 0;

        for (auto l: *page->getLayers()) {
            lua_pushinteger(L, ++currLayer);
            lua_newtable(L);  // beginning of table for layer l

            lua_pushliteral(L, "name");
            lua_pushstring(L, l->getName().c_str());
            lua_settable(L, -3);

            lua_pushliteral(L, "isVisible");
            lua_pushboolean(L, l->isVisible());
            lua_settable(L, -3);

            lua_pushliteral(L, "isAnnotated");
            lua_pushboolean(L, l->isAnnotated());
            lua_settable(L, -3);

            lua_settable(L, -3);  // end of table for layer l
        }
        lua_settable(L, -3);  // end of layers table

        lua_pushliteral(L, "currentLayer");
        lua_pushinteger(L, page->getSelectedLayerId());
        lua_settable(L, -3);

        lua_settable(L, -3);  // end of table for page p
    }
    lua_settable(L, -3);  // end of pages table

    lua_pushliteral(L, "currentPage");
    lua_pushinteger(L, control->getCurrentPageNo() + 1);
    lua_settable(L, -3);

    lua_pushliteral(L, "pdfBackgroundFilename");
    lua_pushstring(L, doc->getPdfFilepath().string().c_str());
    lua_settable(L, -3);

    lua_pushliteral(L, "xoppFilename");
    lua_pushstring(L, doc->getFilepath().string().c_str());
    lua_settable(L, -3);

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

    return 1;
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

    return 1;
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

    return 1;
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
        luaL_error(L, "No page!");
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

    return 1;
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
        luaL_error(L, "No page!");
    }

    size_t layerCount = page->getLayerCount();
    size_t layerId = luaL_checkinteger(L, 1);

    if (layerId > layerCount) {
        luaL_error(L, "No layer with layer ID %d", layerId);
    }

    bool update = false;
    if (lua_isboolean(L, 2)) {
        update = lua_toboolean(L, 2);
    }

    control->getLayerController()->switchToLay(layerId, update);

    return 1;
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
    return 1;
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

    return 1;
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
        luaL_error(L, "No page!");
    }

    if (lua_isstring(L, 1)) {
        auto name = lua_tostring(L, 1);
        page->setBackgroundName(name);
    }

    return 1;
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

    return 1;
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
        luaL_error(L, "Missing output file!");
    }

    fs::path file = fs::path(outputFile);
    auto extension = file.extension();

    if (extension == ".pdf") {
        ExportHelper::exportPdf(doc, outputFile, range, layerRange, bgType, progressiveMode);
    } else if (extension == ".svg" || extension == ".png") {
        ExportHelper::exportImg(doc, outputFile, range, layerRange, pngDpi, pngWidth, pngHeight, bgType);
    }

    // Make sure to remove all vars which are put to the stack before!
    lua_pop(L, 8);

    return 1;
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
 * The full Lua Plugin API.
 * See above for example usage of each function.
 */
static const luaL_Reg applib[] = {{"msgbox", applib_msgbox},
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
                                  {"getFilePath", applib_getFilePath},
                                  {"refreshPage", applib_refreshPage},
                                  {"getStrokes", applib_getStrokes},
                                  {"openFile", applib_openFile},
                                  // Placeholder
                                  //	{"MSG_BT_OK", nullptr},

                                  {nullptr, nullptr}};

/**
 * Open application Library
 */
LUAMOD_API int luaopen_app(lua_State* L) {
    luaL_newlib(L, applib);
    //	lua_pushnumber(L, MSG_BT_OK);
    //	lua_setfield(L, -2, "MSG_BT_OK");
    return 1;
}
