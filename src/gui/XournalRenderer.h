//
// Created by julius on 02.04.20.
//

#pragma once
#include <gtk/gtk.h>

#include "Renderer.h"

class XournalRenderer: public Renderer {
public:
    auto render(cairo_t* cr, Rectangle<double> viewport, double scale) -> void;
    auto getDocumentSize() -> const Rectangle<double>&;
    auto isInfiniteHorizontally() -> bool;
    auto isInfiniteVertically() -> bool;
    auto getGtkStyleContext() -> GtkStyleContext*;
};
