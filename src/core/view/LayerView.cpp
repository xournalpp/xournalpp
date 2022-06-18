#include "LayerView.h"

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include <cairo.h>  // for cairo_clip_extents, cairo_rectangle
#include <glib.h>   // for g_message

#include "model/Element.h"  // for Element
#include "model/Layer.h"    // for Layer

#include "DebugShowRepaintBounds.h"  // for IF_DEBUG_REPAINT
#include "View.h"                    // for Context, ElementView

using namespace xoj::view;

LayerView::LayerView(const Layer* layer): layer(layer) {}

const Layer* LayerView::getLayer() const { return layer; }

void LayerView::draw(const Context& ctx) const {
    IF_DEBUG_REPAINT(int drawn = 0; int notDrawn = 0;);

    // Get the bounds of the mask, in page coordinates
    double minX;
    double maxX;
    double minY;
    double maxY;
    cairo_clip_extents(ctx.cr, &minX, &minY, &maxX, &maxY);

    for (auto& e: layer->getElements()) {

        IF_DEBUG_REPAINT({
            auto cr = ctx.cr;
            cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
            cairo_set_source_rgb(cr, 0, 1, 0);
            cairo_set_line_width(cr, 1);
            cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
            cairo_stroke(cr);
        });

        if (e->intersectsArea(minX, minY, maxX - minX, maxY - minY)) {
            ElementView::createFromElement(e)->draw(ctx);
            IF_DEBUG_REPAINT(drawn++;);
        }
        IF_DEBUG_REPAINT(else { notDrawn++; });
    }
    IF_DEBUG_REPAINT(g_message("DBG:LayerView::draw: draw %i / not draw %i", drawn, notDrawn););
}
