//
// Created by julius on 31.03.20.
//

#pragma once

#include <cairo.h>

#include "Rectangle.h"
#include "Viewport.h"

class Renderer {
public:
    virtual ~Renderer();
    virtual auto render(cairo_t* cr, std::shared_ptr<Viewport> viewport) -> void = 0;
    virtual auto getGtkStyleContext() -> GtkStyleContext* = 0;
};
