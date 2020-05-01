//
// Created by julius on 01.05.20.
//

#pragma once

#include <xournalview/Viewport.h>
using Action = std::variant<ViewportAction>;

struct AppState {
    Viewport viewport;
};
