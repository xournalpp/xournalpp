// luapi_textplaceholder.h
#pragma once
extern "C" {
#include <lua.h>
}

#include "control/Control.h"

class TextPlaceholderConfig;

namespace luapi_textplaceholder {
    int set_placeholder_value(lua_State* L, TextPlaceholderConfig* config, Control* control);
}
