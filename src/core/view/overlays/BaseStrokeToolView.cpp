#include "BaseStrokeToolView.h"

#include <cmath>

#include <cairo.h>

#include "model/Stroke.h"
#include "util/Range.h"
#include "view/Mask.h"
#include "view/Repaintable.h"
#include "view/overlays/OverlayView.h"

using namespace xoj::view;

BaseStrokeToolView::BaseStrokeToolView(Repaintable* parent, const Stroke& stroke):
        ToolView(parent),
        cairoOp(stroke.getToolType() == StrokeTool::HIGHLIGHTER ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER),
        strokeColor(strokeColorWithAlpha(stroke)),
        lineStyle(stroke.getLineStyle()),
        strokeWidth(stroke.getWidth()) {}

BaseStrokeToolView::~BaseStrokeToolView() noexcept = default;

Color BaseStrokeToolView::strokeColorWithAlpha(const Stroke& s) {
    Color c = s.getColor();
    if (s.getToolType() == StrokeTool::HIGHLIGHTER) {
        c.alpha = s.getFill() == -1 ? 120U : static_cast<uint8_t>(s.getFill());
    } else {
        c.alpha = 255U;
    }
    return c;
}

auto BaseStrokeToolView::createMask(cairo_t* tgtcr) const -> Mask {
    const double zoom = this->parent->getZoom();
    Range visibleRange = this->parent->getVisiblePart();

    if (!visibleRange.isValid()) {
        /*
         * The user might be drawing on a page that is not visible at all:
         * e.g. https://github.com/xournalpp/xournalpp/pull/4158#issuecomment-1385954494
         */
        return Mask();
    }

    // Add a small padding, to avoid visual artefacts if scrolling right after finishing a stroke touching the visible
    // area's border
    visibleRange.addPadding(0.5 * this->strokeWidth);

    Mask mask(cairo_get_target(tgtcr), visibleRange, zoom);
    cairo_t* cr = mask.get();

    cairo_set_source_rgba(cr, 1, 1, 1, 1);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    return mask;
}

void BaseStrokeToolView::drawDot(cairo_t* cr, const Point& p) const {
    cairo_set_line_width(cr, p.z == Point::NO_PRESSURE ? this->strokeWidth : p.z);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, p.x, p.y);
    cairo_line_to(cr, p.x, p.y);
    cairo_stroke(cr);
}
