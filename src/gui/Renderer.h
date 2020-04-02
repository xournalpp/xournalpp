//
// Created by julius on 31.03.20.
//

#pragma once

#include <cairo.h>

#include "Rectangle.h"

class Renderer {
public:
    virtual auto render(cairo_t* cr, Rectangle<double> viewport, double scale) -> void;
    virtual auto getDocumentSize() -> const Rectangle<double>&;
    virtual auto isInfiniteHorizontally() -> bool;
    virtual auto isInfiniteVertically() -> bool;
    virtual auto getGtkStyleContext() -> GtkStyleContext*;
};
