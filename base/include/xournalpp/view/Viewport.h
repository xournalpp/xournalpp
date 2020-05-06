//
// Created by julius on 26.04.20.
//

#pragma once

#include <variant>

#include <lager/context.hpp>
struct Viewport {
    int width;
    int height;
    double x;
    double y;
    double rawScale;
};

// Actions
struct Scroll {
    enum Direction { HORIZONTAL, VERTICAL };
    Direction direction;
    double newVal;
};

struct Scale {
    double rawScale;
};

struct Resize {
    int width;
    int height;
};

using ViewportAction = std::variant<Scroll, Resize, Scale>;
using ViewportResult = std::pair<Viewport, lager::effect<ViewportAction>>;

auto viewportUpdate(Viewport model, ViewportAction action) -> ViewportResult;