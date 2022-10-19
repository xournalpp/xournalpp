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

    const double* dashes = nullptr;
    int dashCount = 0;
    stroke.getLineStyle().getDashes(dashes, dashCount);
    assert((dashCount == 0 && dashes == nullptr) || (dashCount != 0 && dashes != nullptr));

    const std::vector<Point>& data = stroke.getPointVector();

    cairo_save(cr);

    if (stroke.hasPressure()) {
        double dashOffset = 0;
        for (const auto& interval: sections) {
            Point p = stroke.getPoint(interval.min);
            cairo_set_line_width(cr, p.z);
            cairo_move_to(cr, p.x, p.y);

            const Point* lastPoint = &p;

            auto endIt = std::next(data.cbegin(), (std::ptrdiff_t)interval.max.index + 1);
            for (auto it = std::next(data.cbegin(), (std::ptrdiff_t)interval.min.index + 1); it != endIt; ++it) {
                if (dashes) {
                    cairo_set_dash(cr, dashes, dashCount, dashOffset);
                    dashOffset += lastPoint->lineLengthTo(*it);
                    lastPoint = &(*it);
                }
                cairo_line_to(cr, it->x, it->y);
                cairo_stroke(cr);
                cairo_set_line_width(cr, it->z);
                cairo_move_to(cr, it->x, it->y);
            }

            if (dashes) {
                cairo_set_dash(cr, dashes, dashCount, dashOffset);
            }

            Point q = stroke.getPoint(interval.max);
            cairo_line_to(cr, q.x, q.y);
            cairo_stroke(cr);
        }
    } else {
        cairo_set_line_width(cr, stroke.getWidth());
        cairo_set_dash(cr, dashes, dashCount, 0);

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
    if (this->erasableStroke.crDebug == nullptr) {
        cairo_matrix_t matrix;
        cairo_get_matrix(cr, &matrix);
        double ratio = matrix.xx;

        // We add a padding
        const double padding = 5 * stroke.getWidth();
        const int width = static_cast<int>(std::ceil((stroke.getElementWidth() + padding) * ratio));
        const int height = static_cast<int>(std::ceil((stroke.getElementHeight() + padding) * ratio));

        this->erasableStroke.surfDebug = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

        cairo_surface_set_device_offset(this->erasableStroke.surfDebug, (0.5 * padding - stroke.getX()) * ratio,
                                        (0.5 * padding - stroke.getY()) * ratio);
        cairo_surface_set_device_scale(this->erasableStroke.surfDebug, ratio, ratio);

        this->erasableStroke.crDebug = cairo_create(this->erasableStroke.surfDebug);

        cairo_set_line_width(this->erasableStroke.crDebug, 2);
    } else {
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

        cairo_set_source_surface(cr, this->erasableStroke.surfDebug, 0, 0);
        cairo_paint(cr);
    }
#endif
    cairo_restore(cr);
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

    cairo_save(cr);

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
        Rectangle<double> box = erasableStroke.getSubSectionBoundingBox(first);
        box.unite(erasableStroke.getSubSectionBoundingBox(last));

        auto [crMask, surfMask] = createMask(box, matrix.xx);
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

        cairo_destroy(crMask);
        crMask = nullptr;

        // Blit the mask
        cairo_mask_surface(cr, surfMask, 0, 0);

        cairo_surface_destroy(surfMask);
        surfMask = nullptr;

        // Avoid drawing those sections again
        ++sectionIt;
        --sectionEndIt;
    }

    for (; sectionIt != sectionEndIt; ++sectionIt) {
        // Create a mask
        auto [crMask, surfMask] = createMask(erasableStroke.getSubSectionBoundingBox(*sectionIt), matrix.xx);
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

        cairo_destroy(crMask);
        crMask = nullptr;

        // Blit the mask
        cairo_mask_surface(cr, surfMask, 0, 0);

        cairo_surface_destroy(surfMask);
        surfMask = nullptr;
    }

    cairo_restore(cr);
}

std::pair<cairo_t*, cairo_surface_t*> ErasableStrokeView::createMask(const Rectangle<double>& box,
                                                                     double scaling) const {
    const int width = static_cast<int>(std::ceil(box.width * scaling));
    const int height = static_cast<int>(std::ceil(box.height * scaling));

    cairo_surface_t* surfMask = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);

    // Apply offset and scaling
    cairo_surface_set_device_offset(surfMask, -box.x * scaling, -box.y * scaling);
    cairo_surface_set_device_scale(surfMask, scaling, scaling);

    // Get a context to draw on our mask
    cairo_t* crMask = cairo_create(surfMask);

    cairo_set_line_join(crMask, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgba(crMask, 1, 1, 1, 1);
    cairo_set_operator(crMask, CAIRO_OPERATOR_SOURCE);
    cairo_set_line_width(crMask, this->erasableStroke.stroke.getWidth());

    return std::make_pair(crMask, surfMask);
}
