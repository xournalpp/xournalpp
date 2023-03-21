#include "ErasableStrokeView.h"

#include <cassert>   // for assert
#include <cmath>     // for ceil
#include <iosfwd>    // for ptrdiff_t
#include <iterator>  // for next
#include <memory>    // for allocator_traits<>::value_type
#include <vector>    // for vector

#include "model/LineStyle.h"              // for LineStyle
#include "model/PathParameter.h"          // for PathParameter
#include "model/Point.h"                  // for Point
#include "model/Stroke.h"                 // for Stroke, StrokeTool::HIGHLIG...
#include "model/eraser/ErasableStroke.h"  // for ErasableStroke, ErasableStr...
#include "util/Color.h"                   // for cairo_set_source_rgbi
#include "util/Interval.h"                // for Interval
#include "util/Rectangle.h"               // for Rectangle
#include "util/Util.h"                    // for cairo_set_dash_from_vector

#include "Mask.h"          // for Mask
#include "StrokeView.h"    // for StrokeView, StrokeView::CAI...
#include "config-debug.h"  // for DEBUG_ERASABLE_STROKE_BOXES

using xoj::util::Rectangle;
using namespace xoj::view;

ErasableStrokeView::ErasableStrokeView(const ErasableStroke& erasableStroke): erasableStroke(erasableStroke) {}

void ErasableStrokeView::draw(cairo_t* cr) const {
    std::vector<ErasableStroke::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    if (sections.empty()) {
        return;
    }

    const Stroke& stroke = this->erasableStroke.stroke;

    const auto& dashes = stroke.getLineStyle().getDashes();

    const std::vector<Point>& data = stroke.getPointVector();

    xoj::util::CairoSaveGuard guard(cr);

    if (stroke.hasPressure()) {
        double dashOffset = 0;
        for (const auto& interval: sections) {
            Point p = stroke.getPoint(interval.min);
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

            Point q = stroke.getPoint(interval.max);
            cairo_line_to(cr, q.x, q.y);
            cairo_stroke(cr);
        }
    } else {
        cairo_set_line_width(cr, stroke.getWidth());
        Util::cairo_set_dash_from_vector(cr, dashes, 0);

        bool mergeFirstAndLast = this->erasableStroke.isClosedStroke() &&
                                 stroke.getToolType() == StrokeTool::HIGHLIGHTER && sections.size() >= 2 &&
                                 sections.front().min == PathParameter(0, 0.0) &&
                                 sections.back().max == PathParameter(data.size() - 2, 1.0);

        auto sectionIt = sections.cbegin();
        auto sectionEndIt = sections.cend();

        if (mergeFirstAndLast) {
            /**
             * Draw the first and last section as one
             */
            const ErasableStroke::SubSection& first = sections.front();
            const ErasableStroke::SubSection& last = sections.back();
            Point p = stroke.getPoint(last.min);
            cairo_move_to(cr, p.x, p.y);

            auto endIt = data.cend();
            for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)last.min.index + 1); it != endIt; ++it) {
                cairo_line_to(cr, it->x, it->y);
            }
            endIt = std::next(data.cbegin(), (std::ptrdiff_t)first.max.index + 1);
            for (auto it = data.cbegin(); it != endIt; ++it) { cairo_line_to(cr, it->x, it->y); }

            Point q = stroke.getPoint(first.max);
            cairo_line_to(cr, q.x, q.y);
            cairo_stroke(cr);

            // Avoid drawing those sections again
            ++sectionIt;
            --sectionEndIt;
        }

        for (; sectionIt != sectionEndIt; ++sectionIt) {
            Point p = stroke.getPoint(sectionIt->min);
            cairo_move_to(cr, p.x, p.y);

            auto endIt = std::next(data.cbegin(), (std::ptrdiff_t)sectionIt->max.index + 1);
            for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)sectionIt->min.index + 1); it != endIt; ++it) {
                cairo_line_to(cr, it->x, it->y);
            }

            Point q = stroke.getPoint(sectionIt->max);
            cairo_line_to(cr, q.x, q.y);
            cairo_stroke(cr);
        }
    }

#ifdef DEBUG_ERASABLE_STROKE_BOXES
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
#endif
}

void ErasableStrokeView::drawFilling(cairo_t* cr) const {
    std::vector<ErasableStroke::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    if (sections.empty()) {
        return;
    }

    const Stroke& stroke = this->erasableStroke.stroke;
    const std::vector<Point>& data = stroke.getPointVector();

    bool mergeFirstAndLast = this->erasableStroke.isClosedStroke() && sections.size() >= 2 &&
                             sections.front().min == PathParameter(0, 0.0) &&
                             sections.back().max == PathParameter(data.size() - 2, 1.0);

    auto sectionIt = sections.cbegin();
    auto sectionEndIt = sections.cend();

    if (mergeFirstAndLast) {
        /**
         * Draw the first and last sections as one
         */
        const ErasableStroke::SubSection& first = sections.front();
        const ErasableStroke::SubSection& last = sections.back();
        Point p = stroke.getPoint(last.min);
        cairo_move_to(cr, p.x, p.y);

        auto endIt = data.cend();
        for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)last.min.index + 1); it != endIt; ++it) {
            cairo_line_to(cr, it->x, it->y);
        }
        endIt = std::next(data.cbegin(), (std::ptrdiff_t)first.max.index + 1);
        for (auto it = data.cbegin(); it != endIt; ++it) { cairo_line_to(cr, it->x, it->y); }

        Point q = stroke.getPoint(first.max);
        cairo_line_to(cr, q.x, q.y);
        cairo_fill(cr);

        // Avoid drawing those sections again
        ++sectionIt;
        --sectionEndIt;
    }

    for (; sectionIt != sectionEndIt; ++sectionIt) {
        Point p = stroke.getPoint(sectionIt->min);
        cairo_move_to(cr, p.x, p.y);

        auto endIt = std::next(data.cbegin(), (std::ptrdiff_t)sectionIt->max.index + 1);
        for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)sectionIt->min.index + 1); it != endIt; ++it) {
            cairo_line_to(cr, it->x, it->y);
        }

        Point q = stroke.getPoint(sectionIt->max);
        cairo_line_to(cr, q.x, q.y);
        cairo_fill(cr);
    }
}

void ErasableStrokeView::paintFilledHighlighter(cairo_t* cr) const {
    const Stroke& stroke = erasableStroke.stroke;
    std::vector<ErasableStroke::SubSection> sections = erasableStroke.getRemainingSubSectionsVector();

    if (sections.empty()) {
        return;
    }

    xoj::util::CairoSaveGuard guard(cr);

    const auto linecap = StrokeView::CAIRO_LINE_CAP[stroke.getStrokeCapStyle()];

    /**
     * We create a mask for each subsection, paint on it and blit them all.
     * We need to rescale the mask according to the scaling of the target cairo context.
     * We find out this scaling by looking at the transformation matrix
     */
    cairo_matrix_t matrix;
    cairo_get_matrix(cr, &matrix);
    // We assume the matrix is an homothety
    assert(matrix.xx == matrix.yy && matrix.xy == 0 && matrix.yx == 0);

    // Initialise the cairo context
    cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
    Util::cairo_set_source_rgbi(cr, stroke.getColor(), static_cast<double>(stroke.getFill()) / 255.0);

    const std::vector<Point>& data = stroke.getPointVector();

    bool mergeFirstAndLast = erasableStroke.isClosedStroke() && sections.size() >= 2 &&
                             sections.front().min == PathParameter(0, 0.0) &&
                             sections.back().max == PathParameter(data.size() - 2, 1.0);

    auto sectionIt = sections.cbegin();
    auto sectionEndIt = sections.cend();

    if (mergeFirstAndLast) {
        /**
         * Draw the first and last sections as one
         */
        const ErasableStroke::SubSection& first = sections.front();
        const ErasableStroke::SubSection& last = sections.back();

        /**
         * Create a mask tailored to the union of the sections' bounding boxes
         */
        Range box = erasableStroke.getSubSectionBoundingBox(first).unite(erasableStroke.getSubSectionBoundingBox(last));

        Mask mask = createMask(cr, box, matrix.xx);
        cairo_t* crMask = mask.get();
        cairo_set_line_cap(crMask, linecap);

        // Paint to the mask
        Point p = stroke.getPoint(last.min);
        cairo_move_to(crMask, p.x, p.y);

        auto endIt = data.cend();
        for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)last.min.index + 1); it != endIt; ++it) {
            cairo_line_to(crMask, it->x, it->y);
        }
        endIt = std::next(data.cbegin(), (std::ptrdiff_t)first.max.index + 1);
        for (auto it = data.cbegin(); it != endIt; ++it) { cairo_line_to(crMask, it->x, it->y); }

        Point q = stroke.getPoint(first.max);
        cairo_line_to(crMask, q.x, q.y);

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
        Point p = stroke.getPoint(sectionIt->min);
        cairo_move_to(crMask, p.x, p.y);

        auto endIt = std::next(data.cbegin(), (std::ptrdiff_t)sectionIt->max.index + 1);
        for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)sectionIt->min.index + 1); it != endIt; ++it) {
            cairo_line_to(crMask, it->x, it->y);
        }

        Point q = stroke.getPoint(sectionIt->max);
        cairo_line_to(crMask, q.x, q.y);

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
