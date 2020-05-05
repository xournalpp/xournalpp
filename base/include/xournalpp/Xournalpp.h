//
// Created by julius on 01.05.20.
//

#pragma once

#include <lager/store.hpp>
#include <xournalpp/settings/Settings.h>

#include "xournalpp/view/Viewport.h"

using Action = std::variant<ViewportAction>;

struct AppState {
    Viewport viewport;
    Settings settings;
};

using XournalppResult = std::pair<AppState, lager::effect<Action>>;
using XournalppStore = lager::store<Action, AppState>;

auto run(int argc, char* argv[]) -> int;