#include "LayerView.h"

#include "model/Layer.h"

using xoj::util::Rectangle;
using namespace xoj::view;

LayerView::LayerView(const Layer* layer): layer(layer) {}

void LayerView::draw(const Context& ctx, const Rectangle<double>& drawArea) const {
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
    int drawn = 0;
    int notDrawn = 0;
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
    for (Element* e: layer->getElements()) {
#ifdef DEBUG_SHOW_ELEMENT_BOUNDS
        auto cr = ctx.cr;
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgb(cr, 0, 1, 0);
        cairo_set_line_width(cr, 1);
        cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
        cairo_stroke(cr);
#endif  // DEBUG_SHOW_REPAINT_BOUNDS

        if (e->intersectsArea(drawArea.x, drawArea.y, drawArea.width, drawArea.height)) {
            auto elementView = ElementView::createFromElement(e);
            elementView->draw(ctx);
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
            drawn++;
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
        }
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
        else {
            notDrawn++;
        }
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
    }
#ifdef DEBUG_SHOW_REPAINT_BOUNDS
    g_message("DBG:DocumentView: draw %i / not draw %i", drawn, notDrawn);
#endif  // DEBUG_SHOW_REPAINT_BOUNDS
}

void LayerView::draw(const Context& ctx) const {
    for (Element* e: layer->getElements()) {
#ifdef DEBUG_SHOW_ELEMENT_BOUNDS
        auto cr = ctx.cr;
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgb(cr, 0, 1, 0);
        cairo_set_line_width(cr, 1);
        cairo_rectangle(cr, e->getX(), e->getY(), e->getElementWidth(), e->getElementHeight());
        cairo_stroke(cr);
#endif  // DEBUG_SHOW_ELEMENT_BOUNDS
        auto elementView = ElementView::createFromElement(e);
        elementView->draw(ctx);
    }
}
