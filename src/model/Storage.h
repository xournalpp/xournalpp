//
// Created by julius on 26.04.20.
//

#pragma once

#include <lager/event_loop/manual.hpp>
#include <lager/store.hpp>
#include <view/Layout.h>
#include <view/Viewport.h>


using Action = std::variant<Scroll, Resize, Scale>;

struct AppState {
    Viewport viewport;
    Layout layout;
};

using State = lager::state<AppState, lager::transactional_tag>;
using Storage = lager::store<Action, State>;

static const Storage storage = lager::make_store<Action>(
        State{},
        [](auto m, auto a) {
            std::visit(lager::visitor{[=](const Resize& resize) { std::pair{m, lager::noop}; }}, a);
        },
        lager::with_manual_event_loop{});