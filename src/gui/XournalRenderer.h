//
// Created by julius on 02.04.20.
//

#pragma once
#include <gtk/gtk.h>

#include "Renderer.h"

class XournalRenderer: public Renderer {
public:
    auto render(cairo_t* cr, std::shared_ptr<Viewport> viewport) -> void override;
    auto getGtkStyleContext() -> GtkStyleContext* override;
};
