#include "BaseStrokeToolView.h"

#include <cmath>

#include <cairo.h>

#include "model/Stroke.h"
#include "util/Range.h"
#include "view/Mask.h"
#include "view/Repaintable.h"
#include "view/overlays/OverlayView.h"

using namespace xoj::view;

static Color strokeColorWithAlpha(const Stroke& s) {
    Color c = s.getColor();
    if (s.getToolType() == StrokeTool::HIGHLIGHTER) {
        c.alpha = s.getFill() == -1 ? 120U : static_cast<uint8_t>(s.getFill());
    } else {
        c.alpha = 255U;
    }
    return c;
}

BaseStrokeToolView::BaseStrokeToolView(Repaintable* parent, const Stroke& stroke):
        ToolView(parent),
        cairoOp(stroke.getToolType() == StrokeTool::HIGHLIGHTER ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER),
        strokeColor(strokeColorWithAlpha(stroke)),
        lineStyle(stroke.getLineStyle()),
        strokeWidth(stroke.getWidth()) {}

BaseStrokeToolView::~BaseStrokeToolView() noexcept = default;

auto BaseStrokeToolView::createMask(cairo_t* tgtcr) const -> Mask {
    const double zoom = this->parent->getZoom();
    const int dpiScaling = this->parent->getDPIScaling();
    Range visibleRange = this->parent->getVisiblePart();

    // Add a small padding, to avoid visual artefacts if scrolling right after finishing a stroke touching the visible
    // area's border
    visibleRange.addPadding(0.5 * this->strokeWidth);

    Mask mask(cairo_get_target(tgtcr), visibleRange, zoom, dpiScaling);
    cairo_t* cr = mask.get();

    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    return mask;
}
