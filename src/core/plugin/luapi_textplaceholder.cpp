#include <lauxlib.h>
// luapi_textplaceholder.cpp
#include "luapi_textplaceholder.h"
#include "config/TextPlaceholderConfig.h"
#include "control/Control.h"
#include "gui/MainWindow.h"
#include "gui/toolbarMenubar/ToolMenuHandler.h"
#include <string>


namespace luapi_textplaceholder {
    int set_placeholder_value(lua_State* L, TextPlaceholderConfig* config, Control* control) {
        // Args: name, value
        const char* name = luaL_checkstring(L, 1);
        const char* value = luaL_checkstring(L, 2);
        config->setValue(name, value);
        config->save();

        // Live update toolbar
        if (control) {
            auto* mainWindow = control->getWindow();
            if (mainWindow) {
                auto* handler = mainWindow->getToolMenuHandler();
                if (handler) {
                    handler->refreshPlaceholderToolItems();
                }
            }
        }
        return 0;
    }
}
