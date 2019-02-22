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

	const char* accelerator = luaL_optstring(L, -3, NULL);
	const char* menu = luaL_optstring(L, -2, NULL);
	const char* callback = luaL_optstring(L, -1, NULL);
	if (callback == NULL)
	{
		luaL_error(L, "Missing callback function!");
	}
	if (menu == NULL)
	{
		menu = "";
	}
	if (accelerator == NULL)
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



static const luaL_Reg applib[] = {
	{ "msgbox", applib_msgbox },
	{ "registerUi", applib_registerUi },

	// Placeholder
//	{"MSG_BT_OK", NULL},
//	{"MSG_BT_YES", NULL},
//	{"MSG_BT_NO", NULL},
//	{"MSG_BT_CANCEL", NULL},

	{NULL, NULL}
};

/**
 * Open application Library
 */
LUAMOD_API int luaopen_app(lua_State* L)
{
	luaL_newlib(L, applib);
//	lua_pushnumber(L, MSG_BT_OK);
//	lua_setfield(L, -2, "MSG_BT_OK");
//	lua_pushnumber(L, MSG_BT_YES);
//	lua_setfield(L, -2, "MSG_BT_YES");
//	lua_pushnumber(L, MSG_BT_NO);
//	lua_setfield(L, -2, "MSG_BT_NO");
//	lua_pushnumber(L, MSG_BT_CANCEL);
//	lua_setfield(L, -2, "MSG_BT_CANCEL");
	return 1;
}

