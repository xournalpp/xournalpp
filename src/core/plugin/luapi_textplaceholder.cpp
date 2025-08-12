#include <lauxlib.h>
// luapi_textplaceholder.cpp
#include "luapi_textplaceholder.h"
#include "config/TextPlaceholderConfig.h"
#include "control/Control.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include <string>

extern Control* g_control;

namespace luapi_textplaceholder {
    int set_placeholder_value(lua_State* L, TextPlaceholderConfig* config) {
        // Args: name, value
        const char* name = luaL_checkstring(L, 1);
        const char* value = luaL_checkstring(L, 2);
        config->setValue(name, value);
        config->save();

        // Live update toolbar
        auto* mainWindow = g_control ? g_control->getWindow() : nullptr;
        if (mainWindow) {
            auto* handler = mainWindow->getToolMenuHandler();
            if (handler) {
                handler->refreshPlaceholderToolItems();
            }
        }
        return 0;
    }
}
