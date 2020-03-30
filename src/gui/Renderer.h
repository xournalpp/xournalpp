//
// Created by julius on 31.03.20.
//

#pragma once

#include "Rectangle.h"

class Renderer {
public:
    virtual auto render(cairo_t* cr, Rectangle<int> viewport, double scale) -> void;
    virtual auto getDocumentSize() -> const Rectangle<int>&;
};
