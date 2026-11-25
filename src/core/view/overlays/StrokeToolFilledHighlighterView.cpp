#include "StrokeToolFilledHighlighterView.h"

#include <vector>

#include "model/Point.h"
#include "util/Color.h"
#include "util/Range.h"
#include "util/raii/CairoWrappers.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

StrokeToolFilledHighlighterView::StrokeToolFilledHighlighterView(const StrokeHandler* strokeHandler,
                                                                 const Stroke& stroke, Repaintable* parent):
        StrokeToolFilledView(strokeHandler, stroke, parent) {}

StrokeToolFilledHighlighterView::~StrokeToolFilledHighlighterView() noexcept = default;

void StrokeToolFilledHighlighterView::draw(cairo_t* cr) const {

    std::vector<Point> pts = this->flushBuffer();
    if (pts.empty()) {
        // The input sequence has probably been cancelled. This view should soon be deleted
        return;
    }

    this->filling.appendSegments(pts);

    if (!this->mask.isInitialized()) {
        // Initialize mask on first call
        this->mask = this->createMask(cr);
        if (!mask.isInitialized()) {
            /*
             * The user might be drawing on a page that is not visible at all:
             * e.g. https://github.com/xournalpp/xournalpp/pull/4158#issuecomment-1385954494
             */
            return;
        }
    }

    if (this->singleDot) {
        this->drawDot(this->mask.get(), pts.back());
    } else {
        /*
         * Upon adding a segment, the filling can actually shrink.
         * We wipe the portion of the mask that can change: the convex hull of the added points + the first point
         */
        Range wipe(filling.firstPoint.x, filling.firstPoint.y);

        for (auto& p: pts) {
            wipe.addPoint(p.x, p.y);
        }

        if (wipe.isValid()) {
            this->mask.wipeRange(wipe);
        }


        /*
         * Draw both the filling and the stroke alike on the mask
         */
        cairo_set_line_width(this->mask.get(), this->strokeWidth);
        StrokeViewHelper::pathToCairo(this->mask.get(), this->filling.contour);
        cairo_fill_preserve(this->mask.get());
        cairo_stroke(this->mask.get());
    }

    xoj::util::CairoSaveGuard saveGuard(cr);
    Util::cairo_set_source_argb(cr, this->strokeColor);
    cairo_set_operator(cr, this->cairoOp);

    this->mask.blitTo(cr);
}
