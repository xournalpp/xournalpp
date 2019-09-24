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

#include <XojMsgBox.h>
#include <StringUtils.h>

#include "control/Control.h"
#include "control/PageBackgroundChangeController.h"
#include "control/pagetype/PageTypeHandler.h"

#include <map>
using std::map;

/**
 * Example:
 * app.msgbox("Test123", {[1] = "Yes", [2] = "No"})
 * Return 1 for yes, 2 for no in this example
 */
static int applib_msgbox(lua_State* L)
{
	const char* msg = luaL_checkstring(L, 1);

	// discard any extra arguments passed in
	lua_settop(L, 2);
	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushnil(L);

	map<int, string> button;

	while (lua_next(L, 2) != 0)
	{
		int index = lua_tointeger(L, -2);
		const char* buttonText = luaL_checkstring(L, -1);
		lua_pop(L, 1);

		button.insert(button.begin(), std::pair<int, string>(index, buttonText));
	}

	Plugin* plugin = Plugin::getPluginFromLua(L);

	int result = XojMsgBox::showPluginMessage(plugin->getName(), msg, button);
	lua_pushinteger(L, result);
	return 1;
}


/**
 * Allow to register menupoints or toolbar icons, this needs to be called from initUi
 */
static int applib_registerUi(lua_State* L)
{
	Plugin* plugin = Plugin::getPluginFromLua(L);
	if (!plugin->isInInitUi())
	{
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
	// stack now has following:
	//    1 = {"menu"="MenuName", callback="functionName", accelerator="<Control>a"}
	//   -3 = "<Control>a"
	//   -2 = "MenuName"
	//   -1 = "functionName"

	const char* accelerator = luaL_optstring(L, -3, nullptr);
	const char* menu = luaL_optstring(L, -2, nullptr);
	const char* callback = luaL_optstring(L, -1, nullptr);
	if (callback == nullptr)
	{
		luaL_error(L, "Missing callback function!");
	}
	if (menu == nullptr)
	{
		menu = "";
	}
	if (accelerator == nullptr)
	{
		accelerator = "";
	}

	int menuId = -1;
	int toolbarId = -1;

	if (menu)
	{
		menuId = plugin->registerMenu(menu, callback, accelerator);
	}

	// Make sure to remove all vars which are put to the stack before!
	lua_pop(L, 3);

	// Add return value to the Stack
	lua_createtable(L, 0, 2);

	lua_pushstring(L, "menuId");
	lua_pushinteger(L, menuId);
	lua_settable(L, -3);  /* 3rd element from the stack top */

	lua_pushstring(L, "toolbarId");
	lua_pushinteger(L, toolbarId);
	lua_settable(L, -3);

	return 1;
}

/**
 * Execute an UI action (usually internal called from Toolbar / Menu)
 */
static int applib_uiAction(lua_State* L)
{
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
	if (groupStr != nullptr)
	{
		group = ActionGroup_fromString(groupStr);
	}

	if (lua_isboolean(L, -2))
	{
		enabled = lua_toboolean(L, -2);
	}

	const char* actionStr = luaL_optstring(L, -1, nullptr);
	if (actionStr == nullptr)
	{
		luaL_error(L, "Missing action!");
	}

	ActionType action = ActionType_fromString(actionStr);
	GdkEvent* event = nullptr;
	GtkMenuItem* menuitem = nullptr;
	GtkToolButton* toolbutton = nullptr;

	Control* ctrl = plugin->getControl();
	ctrl->actionPerformed(action, group, event, menuitem, toolbutton, enabled);

	// Make sure to remove all vars which are put to the stack before!
	lua_pop(L, 3);

	return 1;
}

/**
 * Select UI action
 */
static int applib_uiActionSelected(lua_State* L)
{
	Plugin* plugin = Plugin::getPluginFromLua(L);

	ActionGroup group = group = ActionGroup_fromString(luaL_checkstring(L, 1));
	ActionType action = ActionType_fromString(luaL_checkstring(L, 2));

	Control* ctrl = plugin->getControl();
	ctrl->fireActionSelected(group, action);

	return 1;
}

/**
 * Select UI action
 */
static int applib_changeCurrentPageBackground(lua_State* L)
{
	PageType pt;
	pt.format = PageTypeHandler::getPageTypeFormatForString(luaL_checkstring(L, 1));
	pt.config = luaL_optstring(L, 2, "");

	Plugin* plugin = Plugin::getPluginFromLua(L);
	Control* ctrl = plugin->getControl();
	PageBackgroundChangeController* pageBgCtrl = ctrl->getPageBackgroundChangeController();
	pageBgCtrl->changeCurrentPageBackground(pt);

	return 1;
}


static const luaL_Reg applib[] = {
	{ "msgbox", applib_msgbox },
	{ "registerUi", applib_registerUi },
	{ "uiAction", applib_uiAction },
	{ "uiActionSelected", applib_uiActionSelected },
	{ "changeCurrentPageBackground", applib_changeCurrentPageBackground },

	// Placeholder
//	{"MSG_BT_OK", nullptr},

	{nullptr, nullptr}
};

/**
 * Open application Library
 */
LUAMOD_API int luaopen_app(lua_State* L)
{
	luaL_newlib(L, applib);
//	lua_pushnumber(L, MSG_BT_OK);
//	lua_setfield(L, -2, "MSG_BT_OK");
	return 1;
}

