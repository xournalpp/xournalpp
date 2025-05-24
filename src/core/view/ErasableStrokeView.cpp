#include "ErasableStrokeView.h"

#include <cmath>     // for ceil
#include <iosfwd>    // for ptrdiff_t
#include <iterator>  // for next
#include <memory>    // for allocator_traits<>::value_type
#include <vector>    // for vector

#include "model/LineStyle.h"  // for LineStyle
#include "model/Point.h"      // for Point
#include "model/Stroke.h"     // for Stroke, StrokeTool::HIGHLIG...
#include "model/eraser/ErasablePressureSpline.h"
#include "model/eraser/ErasableStroke.h"     // for ErasableStroke, ErasableStr...
#include "model/path/Path.h"                 // for Path
#include "model/path/PiecewiseLinearPath.h"  // for PiecewiseLinearPath
#include "model/path/Spline.h"               // for Spline
#include "util/Assert.h"                     // for xoj_assert
#include "util/Color.h"                      // for cairo_set_source_rgbi
#include "util/Interval.h"                   // for Interval
#include "util/Rectangle.h"                  // for Rectangle
#include "util/Util.h"                       // for cairo_set_dash_from_vector

#include "Mask.h"          // for Mask
#include "StrokeView.h"    // for StrokeView, StrokeView::CAI...
#include "config-debug.h"  // for DEBUG_ERASABLE_STROKE_BOXES

using xoj::util::Rectangle;
using namespace xoj::view;

ErasableStrokeView::ErasableStrokeView(const ErasableStroke& erasableStroke): erasableStroke(erasableStroke) {}

void ErasableStrokeView::draw(cairo_t* cr) const {
    if (this->erasableStroke.getType() == ErasableStroke::PRESSURE_SPLINE) {
        drawPressureSpline(cr);
        return;
    }

    std::vector<Path::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    if (sections.empty()) {
        return;
    }

    const Stroke& stroke = this->erasableStroke.stroke;
    const Path& path = stroke.getPath();

    const auto& dashes = stroke.getLineStyle().getDashes();

    xoj::util::CairoSaveGuard guard(cr);

    if (stroke.hasPressure()) {
        /**
         * No need to merge the first and last sections here (if the stroke is closed).
         * This merging is only necessary for highlighter strokes, which are not pressure sensitive.
         */
        const std::vector<Point>& data = path.getData();
        double dashOffset = 0;
        for (const auto& interval: sections) {
            Point p = path.getPoint(interval.min);
            cairo_set_line_width(cr, p.z);
            cairo_move_to(cr, p.x, p.y);

            const Point* lastPoint = &p;

            auto endIt = std::next(data.cbegin(), (std::ptrdiff_t)interval.max.index + 1);
            for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)interval.min.index + 1); it != endIt; ++it) {
                if (!dashes.empty()) {
                    Util::cairo_set_dash_from_vector(cr, dashes, dashOffset);
                    dashOffset += lastPoint->lineLengthTo(*it);
                    lastPoint = &(*it);
                }
                cairo_line_to(cr, it->x, it->y);
                cairo_stroke(cr);
                cairo_set_line_width(cr, it->z);
                cairo_move_to(cr, it->x, it->y);
            }

            if (!dashes.empty()) {
                Util::cairo_set_dash_from_vector(cr, dashes, dashOffset);
            }

            Point q = path.getPoint(interval.max);
            cairo_line_to(cr, q.x, q.y);
            cairo_stroke(cr);
        }
    } else {
        cairo_set_line_width(cr, stroke.getWidth());
        Util::cairo_set_dash_from_vector(cr, dashes, 0);

        bool mergeFirstAndLast = this->erasableStroke.isClosedStroke() &&
                                 stroke.getToolType() == StrokeTool::HIGHLIGHTER && sections.size() >= 2 &&
                                 sections.front().min == Path::Parameter(0, 0.0) &&
                                 sections.back().max == Path::Parameter(path.nbSegments() - 1, 1.0);

        auto sectionIt = sections.cbegin();
        auto sectionEndIt = sections.cend();

        if (mergeFirstAndLast) {
            /**
             * Draw the first and last section as one
             */
            path.addCircularSectionToCairo(cr, sections.back().min, sections.front().max);
            cairo_stroke(cr);

            // Avoid drawing those sections again
            ++sectionIt;
            --sectionEndIt;
        }

        for (; sectionIt != sectionEndIt; ++sectionIt) {
            path.addSectionToCairo(cr, *sectionIt);
            cairo_stroke(cr);
        }
    }

#ifdef DEBUG_ERASABLE_STROKE_BOXES
    blitDebugMask(cr);
#endif
}

void ErasableStrokeView::drawPressureSpline(cairo_t* cr) const {
    std::vector<Path::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    auto pointCache = dynamic_cast<const ErasablePressureSpline&>(this->erasableStroke).pointCache;
    if (sections.empty()) {
        pointCache.clear();
        return;
    }

    const Stroke& stroke = this->erasableStroke.stroke;
    const Path& path = stroke.getPath();

    auto& dashes = stroke.getLineStyle().getDashes();

    for (auto& interval: sections) {
        Point p = path.getPoint(interval.min);
        cairo_set_line_width(cr, p.z);
        cairo_move_to(cr, p.x, p.y);

        double dashOffset = 0.0;
        const Point* lastPoint = &p;

        auto pointCache = dynamic_cast<const ErasablePressureSpline&>(this->erasableStroke).pointCache;

        auto pointCacheIt = pointCache.cbegin() + static_cast<std::ptrdiff_t>(interval.min.index);

        // Iterator to the first point whose parameter is greater that interval.min.t
        auto ptIt =
                std::upper_bound(pointCacheIt->cbegin(), pointCacheIt->cend(), interval.min.t,
                                 [](const double& t, const SplineSegment::ParametrizedPoint& pt) { return t < pt.t; });

        auto endPointCacheIt = pointCache.cbegin() + static_cast<std::ptrdiff_t>(interval.max.index);
        if (pointCacheIt != endPointCacheIt) {
            // The drawn interval spans more than one spline segment
            for (auto endPtIt = pointCacheIt->cend(); ptIt < endPtIt; ++ptIt) {
                // First (portion of a) segment
                if (!dashes.empty()) {
                    Util::cairo_set_dash_from_vector(cr, dashes, dashOffset);
                    dashOffset += lastPoint->lineLengthTo(*ptIt);
                    lastPoint = &(*ptIt);
                }
                cairo_line_to(cr, ptIt->x, ptIt->y);
                cairo_stroke(cr);
                cairo_set_line_width(cr, ptIt->z);
                cairo_move_to(cr, ptIt->x, ptIt->y);
            }
            ++pointCacheIt;
            for (; pointCacheIt != endPointCacheIt; ++pointCacheIt) {
                // Middle segments
                for (auto& p: *pointCacheIt) {
                    if (!dashes.empty()) {
                        Util::cairo_set_dash_from_vector(cr, dashes, dashOffset);
                        dashOffset += lastPoint->lineLengthTo(p);
                        lastPoint = &p;
                    }
                    cairo_line_to(cr, p.x, p.y);
                    cairo_stroke(cr);
                    cairo_set_line_width(cr, p.z);
                    cairo_move_to(cr, p.x, p.y);
                }
            }
            ptIt = pointCacheIt->cbegin();
        }
        // Iterator to the first point whose parameter it greater or equal to interval.max.t
        auto endPtIt =
                std::lower_bound(ptIt, pointCacheIt->cend(), interval.max.t,
                                 [](const SplineSegment::ParametrizedPoint& pt, const double& t) { return pt.t < t; });

        for (; ptIt < endPtIt; ++ptIt) {
            // Last (portion of a) segment
            if (!dashes.empty()) {
                Util::cairo_set_dash_from_vector(cr, dashes, dashOffset);
                dashOffset += lastPoint->lineLengthTo(*ptIt);
                lastPoint = &(*ptIt);
            }
            cairo_line_to(cr, ptIt->x, ptIt->y);
            cairo_stroke(cr);
            cairo_set_line_width(cr, ptIt->z);
            cairo_move_to(cr, ptIt->x, ptIt->y);
        }

        if (!dashes.empty()) {
            Util::cairo_set_dash_from_vector(cr, dashes, dashOffset);
        }
        Point q = path.getPoint(interval.max);
        cairo_line_to(cr, q.x, q.y);
        cairo_stroke(cr);
    }

#ifdef DEBUG_ERASABLE_STROKE_BOXES
    blitDebugMask(cr);
#endif
}


void ErasableStrokeView::drawFilling(cairo_t* cr) const {
    std::vector<Path::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    if (sections.empty()) {
        return;
    }

    const Stroke& stroke = this->erasableStroke.stroke;
    const Path& path = stroke.getPath();

    auto sectionIt = sections.cbegin();
    auto sectionEndIt = sections.cend();

    const bool mergeFirstAndLast = this->erasableStroke.isClosedStroke() && sections.size() >= 2 &&
                                   sections.front().min == Path::Parameter(0, 0.0) &&
                                   sections.back().max == Path::Parameter(path.nbSegments() - 1, 1.0);

    if (mergeFirstAndLast) {
        /**
         * Draw the first and last section as one
         */
        path.addCircularSectionToCairo(cr, sections.back().min, sections.front().max);
        cairo_fill(cr);

        // Avoid drawing those sections again
        ++sectionIt;
        --sectionEndIt;
    }

    for (; sectionIt != sectionEndIt; ++sectionIt) {
        path.addSectionToCairo(cr, *sectionIt);
        cairo_fill(cr);
    }
}

void ErasableStrokeView::paintFilledHighlighter(cairo_t* cr) const {
    std::vector<Path::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    if (sections.empty()) {
        return;
    }

    xoj::util::CairoSaveGuard guard(cr);

    const auto linecap = StrokeView::CAIRO_LINE_CAP[erasableStroke.stroke.getStrokeCapStyle()];

    /**
     * We create a mask for each subsection, paint on it and blit them all.
     * We need to rescale the mask according to the scaling of the target cairo context.
     * We find out this scaling by looking at the transformation matrix
     */
    cairo_matrix_t matrix;
    cairo_get_matrix(cr, &matrix);
    // We assume the matrix is an homothety
    xoj_assert(matrix.xx == matrix.yy && matrix.xy == 0 && matrix.yx == 0);

    const Stroke& stroke = erasableStroke.stroke;
    const Path& path = stroke.getPath();

    // Initialise the cairo context
    cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
    Util::cairo_set_source_rgbi(cr, stroke.getColor(), static_cast<double>(stroke.getFill()) / 255.0);

    const bool mergeFirstAndLast = erasableStroke.isClosedStroke() && sections.size() >= 2 &&
                                   sections.front().min == Path::Parameter(0, 0.0) &&
                                   sections.back().max == Path::Parameter(path.nbSegments() - 1, 1.0);

    auto sectionIt = sections.cbegin();
    auto sectionEndIt = sections.cend();

    if (mergeFirstAndLast) {
        /**
         * Draw the first and last section as one
         */
        const Path::SubSection& first = sections.front();
        const Path::SubSection& last = sections.back();

        /**
         * Create a mask tailored to the union of the sections' bounding boxes
         */
        Range box = erasableStroke.getSubSectionBoundingBox(first).unite(erasableStroke.getSubSectionBoundingBox(last));

        Mask mask = createMask(cr, box, matrix.xx);
        cairo_t* crMask = mask.get();
        cairo_set_line_cap(crMask, linecap);

        // Paint to the mask
        path.addCircularSectionToCairo(crMask, last.min, first.max);

        cairo_fill_preserve(crMask);
        cairo_stroke(crMask);

        mask.blitTo(cr);

        // Avoid drawing those sections again
        ++sectionIt;
        --sectionEndIt;
    }

    for (; sectionIt != sectionEndIt; ++sectionIt) {
        // Create a mask
        Mask mask = createMask(cr, erasableStroke.getSubSectionBoundingBox(*sectionIt), matrix.xx);
        cairo_t* crMask = mask.get();
        cairo_set_line_cap(crMask, linecap);

        // Paint to the mask
        path.addSectionToCairo(crMask, *sectionIt);

        cairo_fill_preserve(crMask);
        cairo_stroke(crMask);

        mask.blitTo(cr);
    }
}

auto ErasableStrokeView::createMask(cairo_t* target, const Range& box, double zoom) const -> Mask {
    Mask mask(cairo_get_target(target), box, zoom);

    cairo_t* crMask = mask.get();
    cairo_set_line_join(crMask, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgba(crMask, 1, 1, 1, 1);
    cairo_set_operator(crMask, CAIRO_OPERATOR_SOURCE);
    cairo_set_line_width(crMask, this->erasableStroke.stroke.getWidth());

    return mask;
}

#ifdef DEBUG_ERASABLE_STROKE_BOXES
void ErasableStrokeView::blitDebugMask(cairo_t* cr) const {
    const Stroke& stroke = this->erasableStroke.stroke;
    if (!this->erasableStroke.debugMask.isInitialized()) {
        cairo_matrix_t matrix;
        cairo_get_matrix(cr, &matrix);

        Range extents(0, 0, stroke.getElementWidth(), stroke.getElementHeight());
        extents.addPadding(5 * stroke.getWidth());
        erasableStroke.debugMask = Mask(cairo_get_target(cr), extents, matrix.xx, CAIRO_CONTENT_COLOR_ALPHA);
        cairo_set_line_width(this->erasableStroke.debugMask.get(), 2);
    } else {
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        this->erasableStroke.debugMask.paintTo(cr);
    }
}
#endif
