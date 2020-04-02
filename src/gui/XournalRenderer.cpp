//
// Created by julius on 02.04.20.
//

#include "XournalRenderer.h"

auto XournalRenderer::render(cairo_t* cr, Rectangle<double> viewport, double scale) -> void {}

auto XournalRenderer::getDocumentSize() -> const Rectangle<double>& {};
auto XournalRenderer::isInfiniteHorizontally() -> bool{};
auto XournalRenderer::isInfiniteVertically() -> bool{};
auto XournalRenderer::getGtkStyleContext() -> GtkStyleContext* {};