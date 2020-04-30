//
// Created by julius on 26.04.20.
//

#pragma once

#include <control/Action.h>
#include <lager/event_loop/manual.hpp>
#include <lager/store.hpp>
#include <model/softstorage/Layout.h>
#include <model/softstorage/Viewport.h>

struct AppState {
    Viewport viewport;
    Layout layout;
};

using State = lager::state<AppState, lager::transactional_tag>;
using Storage = lager::store<Action, State>;

static const Storage storage = lager::make_store<Action>(
        State{},
        [](auto m, auto a) {
            return std::pair{m + a, [&](auto context) {}};
        },
        lager::with_manual_event_loop{});