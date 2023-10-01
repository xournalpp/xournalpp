#include "StrokeToolFilledView.h"

#include <algorithm>

#include "model/Stroke.h"
#include "util/Assert.h"
#include "util/Color.h"
#include "util/Range.h"
#include "view/Repaintable.h"
#include "view/StrokeViewHelper.h"

using namespace xoj::view;

static const Point& setupFirstPoint(const Stroke& s) {
    xoj_assert(s.hasPath());
    const auto& path = s.getPath();
    xoj_assert(!path.empty());
    return path.getFirstKnot();
}

StrokeToolFilledView::StrokeToolFilledView(const StrokeHandler* strokeHandler, const Stroke& stroke,
                                           Repaintable* parent):
        StrokeToolView(strokeHandler, stroke, parent), filling(stroke.getFill() / 255.0, setupFirstPoint(stroke)) {}

StrokeToolFilledView::~StrokeToolFilledView() noexcept = default;

void StrokeToolFilledView::drawFilling(cairo_t* cr, const PiecewiseLinearPath& pts) const {
    this->filling.appendSegments(pts);
    /*
     * Draw the filling directly (not through the mask)
     *   Upon adding a segment, the filling can actually shrink, making it easier to redraw the filling every time.
     */
    this->filling.contour.addToCairo(cr);
    Util::cairo_set_source_rgbi(cr, strokeColor, this->filling.alpha);
    cairo_fill(cr);
}

void StrokeToolFilledView::on(StrokeToolView::AddPointRequest, const Point& p) {
    this->singleDot = false;
    xoj_assert(!this->pointBuffer.empty());
    Range rg = getRepaintRange(this->pointBuffer.getLastKnot(), p);
    this->pointBuffer.addLineSegmentTo(p);
    // Add the first point, so that the range covers all the filling changes
    rg.addPoint(this->filling.firstPoint.x, this->filling.firstPoint.y);
    this->parent->flagDirtyRegion(rg);
}

void StrokeToolFilledView::on(StrokeToolView::StrokeReplacementRequest, const Stroke& newStroke) {
    StrokeToolView::on(STROKE_REPLACEMENT_REQUEST, newStroke);
    this->filling.contour = this->pointBuffer;
    if (!this->pointBuffer.empty()) {
        const Point& fp = this->pointBuffer.getFirstKnot();
        this->filling.firstPoint = utl::Point<double>(fp.x, fp.y);
    }
}

void StrokeToolFilledView::FillingData::appendSegments(const PiecewiseLinearPath& pts) { contour.append(pts); }
