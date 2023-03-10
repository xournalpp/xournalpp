#include "StrokeViewHelper.h"

#include <cassert>

#include "model/LineStyle.h"
#include "model/Point.h"
#include "util/LoopUtil.h"
#include "util/PairView.h"

void xoj::view::StrokeViewHelper::pathToCairo(cairo_t* cr, const std::vector<Point>& pts) {
    for_first_then_each(
            pts, [cr](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [cr](auto const& other) { cairo_line_to(cr, other.x, other.y); });
}

/**
 * No pressure sensitivity, one line is drawn
 */
void xoj::view::StrokeViewHelper::drawNoPressure(cairo_t* cr, const std::vector<Point>& pts, const double strokeWidth,
                                                 const LineStyle& lineStyle, double dashOffset) {
    cairo_set_line_width(cr, strokeWidth);

    const auto& dashes = lineStyle.getDashes();
    cairo_set_dash(cr, dashes.data(), static_cast<int>(dashes.size()), dashOffset);

    pathToCairo(cr, pts);
    cairo_stroke(cr);
}

/**
 * Draw a stroke with pressure, for this multiple lines with different widths needs to be drawn
 */
double xoj::view::StrokeViewHelper::drawWithPressure(cairo_t* cr, const std::vector<Point>& pts,
                                                     const LineStyle& lineStyle, double dashOffset) {
    const auto& dashes = lineStyle.getDashes();

    /*
     * Because the width varies, we need to call cairo_stroke() once per segment
     */
    auto drawSegment = [cr](const Point& p, const Point& q) {
        assert(p.z > 0.0);
        cairo_set_line_width(cr, p.z);
        cairo_move_to(cr, p.x, p.y);
        cairo_line_to(cr, q.x, q.y);
        cairo_stroke(cr);
    };

    if (!dashes.empty()) {
        for (const auto& [p, q]: PairView(pts)) {
            cairo_set_dash(cr, dashes.data(), static_cast<int>(dashes.size()), dashOffset);
            dashOffset += p.lineLengthTo(q);
            drawSegment(p, q);
        }
    } else {
        cairo_set_dash(cr, nullptr, 0, 0.0);
        for (const auto& [p, q]: PairView(pts)) {
            drawSegment(p, q);
        }
    }
    return dashOffset;
}
