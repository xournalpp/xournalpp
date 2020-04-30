//
// Created by julius on 31.03.20.
//

#pragma once

#include <cairo.h>

#include "model/softstorage/Viewport.h"

#include "Rectangle.h"

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual auto render(cairo_t* cr) -> void = 0;
    virtual auto getGtkStyleContext() -> GtkStyleContext* = 0;
};
