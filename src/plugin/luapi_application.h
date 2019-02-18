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

/**
 * Example:
 * app.msgbox("Test123", "yes,no")
 */
static int applib_msgbox(lua_State* L)
{
	const char* msg = luaL_checkstring(L, 1);
	const char* flags = luaL_checkstring(L, 2);

	int type = 0;
	for (string element : StringUtils::split(flags, ','))
	{
		if (element == "ok")
		{
			type |= MSG_BT_OK;
		}
		else if (element == "yes")
		{
			type |= MSG_BT_YES;
		}
		else if (element == "no")
		{
			type |= MSG_BT_NO;
		}
		else if (element == "cancel")
		{
			type |= MSG_BT_CANCEL;
		}
		else
		{
			g_warning("Plugin: Unsupported button type for app.msgbox «%s»", element.c_str());
		}
	}

	Plugin* plugin = Plugin::getPluginFromLua(L);

	int result = XojMsgBox::showPluginMessage(plugin->getName(), msg, type);
	lua_pushinteger(L, result);
	return 1;
}


static const luaL_Reg applib[] = {
  {"msgbox",   applib_msgbox},

  // Placeholder
  {"MSG_BT_OK", NULL},
  {"MSG_BT_YES", NULL},
  {"MSG_BT_NO", NULL},
  {"MSG_BT_CANCEL", NULL},

  {NULL, NULL}
};

/**
 * Open application Library
 */
LUAMOD_API int luaopen_app(lua_State* L)
{
	luaL_newlib(L, applib);
	lua_pushnumber(L, MSG_BT_OK);
	lua_setfield(L, -2, "MSG_BT_OK");
	lua_pushnumber(L, MSG_BT_YES);
	lua_setfield(L, -2, "MSG_BT_YES");
	lua_pushnumber(L, MSG_BT_NO);
	lua_setfield(L, -2, "MSG_BT_NO");
	lua_pushnumber(L, MSG_BT_CANCEL);
	lua_setfield(L, -2, "MSG_BT_CANCEL");
	return 1;
}

