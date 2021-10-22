#include "StrokeView.h"

#include <cmath>

#include "model/Stroke.h"
#include "model/eraser/ErasableStroke.h"
#include "util/LoopUtil.h"

#include "DocumentView.h"

using xoj::util::Rectangle;
using namespace xoj::view;

StrokeView::StrokeView(const Stroke* s): s(s) {}

void StrokeView::pathToCairo(cairo_t* cr) const {
    for_first_then_each(
            s->getPointVector(), [cr](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [cr](auto const& other) { cairo_line_to(cr, other.x, other.y); });
}

void StrokeView::drawErasableStroke(cairo_t* cr, const Stroke* s) {
    ErasableStroke* e = s->getErasable();
    e->draw(cr);
}

/**
 * No pressure sensitivity, one line is drawn
 */
void StrokeView::drawNoPressure(cairo_t* cr) const {
    cairo_set_line_width(cr, s->getWidth());

    const double* dashes = nullptr;
    int dashCount = 0;
    s->getLineStyle().getDashes(dashes, dashCount);
    assert((dashCount == 0 && dashes == nullptr) || (dashCount != 0 && dashes != nullptr));
    cairo_set_dash(cr, dashes, dashCount, 0);

    pathToCairo(cr);
    cairo_stroke(cr);
}

/**
 * Draw a stroke with pressure, for this multiple lines with different widths needs to be drawn
 */
void StrokeView::drawWithPressure(cairo_t* cr) const {
    double dashOffset = 0;
    const double* dashes = nullptr;
    int dashCount = 0;
    s->getLineStyle().getDashes(dashes, dashCount);
    assert((dashCount == 0 && dashes == nullptr) || (dashCount != 0 && dashes != nullptr));

    for (auto p1i = begin(s->getPointVector()), p2i = std::next(p1i), endi = end(s->getPointVector());
         p1i != endi && p2i != endi; ++p1i, ++p2i) {
        auto width = p1i->z != Point::NO_PRESSURE ? p1i->z : s->getWidth();
        cairo_set_line_width(cr, width);
        if (dashes) {
            cairo_set_dash(cr, dashes, dashCount, dashOffset);
            dashOffset += p1i->lineLengthTo(*p2i);
        }
        cairo_move_to(cr, p1i->x, p1i->y);
        cairo_line_to(cr, p2i->x, p2i->y);
        cairo_stroke(cr);
    }
}

void StrokeView::draw(const Context& ctx) const {

    if (s->getPointCount() < 2) {
        // Should not happen
        g_warning("View::StrokeView::draw empty stroke...");
        return;
    }

    cairo_save(ctx.cr);

    const bool highlighter = s->getToolType() == STROKE_TOOL_HIGHLIGHTER;
    const bool filledHighlighter = highlighter && s->getFill() != -1;
    const bool drawTranslucent = ctx.fadeOutNonAudio && s->getAudioFilename().empty();
    const bool useMask = (!ctx.noColor && filledHighlighter) || drawTranslucent;

    // The mask will be colorblind
    const bool noColor = ctx.noColor || useMask;

    cairo_surface_t* surfMask = nullptr;

    // If not using a mask, draw directly onto the given cairo context
    cairo_t* cr = ctx.cr;

    if (useMask) {
        /**
         * To avoid visual glitches when different translucent cairo_stroke are painted,
         * they are painted without colors to a mask which will in turn be blitted (see below)
         */

        /**
         * We need to rescale the mask according to the scaling ratio of the target cairo context.
         * We find out this scaling by looking at the transformation matrix
         */
        cairo_matrix_t matrix;
        cairo_get_matrix(ctx.cr, &matrix);
        // We assume the matrix is diagonal (i.e. only scaling, no rotation)
        assert(matrix.xy == 0 && matrix.yx == 0);

        /**
         * Create a surface tailored to the stroke's bounding box
         */
        Rectangle<double> box = s->boundingRect();

        const int width = static_cast<int>(std::ceil(box.width * matrix.xx));
        const int height = static_cast<int>(std::ceil(box.height * matrix.yy));

        surfMask = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);

        // Apply offset and scaling
        cairo_surface_set_device_offset(surfMask, -box.x * matrix.xx, -box.y * matrix.yy);
        cairo_surface_set_device_scale(surfMask, matrix.xx, matrix.yy);

        // Get a context to draw on our mask
        cr = cairo_create(surfMask);

#ifdef DEBUG_SHOW_MASK
        cairo_set_source_rgba(cr, 1, 1, 1, 0.3);
        cairo_paint(cr);
#endif
    }

    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    cairo_line_cap_t capType;
    switch (s->getStrokeCapStyle()) {
        case StrokeCapStyle::BUTT:
            capType = CAIRO_LINE_CAP_BUTT;
            break;
        case StrokeCapStyle::SQUARE:
            capType = CAIRO_LINE_CAP_SQUARE;
            break;
        default:
            capType = CAIRO_LINE_CAP_ROUND;
    }
    cairo_set_line_cap(cr, capType);

    if (auto fill = s->getFill(); fill != -1) {
        /**
         * Paint the filling
         */
        if (noColor) {
            // Painting on an Alpha-only mask. No need for colors.
            if (filledHighlighter) {
                cairo_set_source_rgba(cr, 1, 1, 1, 1);
            } else {
                cairo_set_source_rgba(cr, 1, 1, 1, static_cast<double>(fill) / 255.0);
            }
        } else {
            DocumentView::applyColor(cr, s, static_cast<uint8_t>(fill));
        }
        cairo_set_operator(cr, useMask ? CAIRO_OPERATOR_SOURCE : CAIRO_OPERATOR_OVER);
        pathToCairo(cr);

        cairo_fill(cr);
    }

    /**
     * Picking the colors and alpha values, and the operator
     */
    if (noColor) {
        /**
         * No color: painting onto an alpha mask. Colors will be applied upon blitting.
         */
        cairo_set_source_rgba(cr, 1, 1, 1, 1);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    } else if (highlighter) {
        /**
         * Highlighter without filling.
         */
        DocumentView::applyColor(cr, s, HIGHLIGHTER_ALPHA);
        cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
    } else {
        /**
         * Normal pen
         */
        DocumentView::applyColor(cr, static_cast<const Element*>(s));
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }

    if (s->getErasable() && ctx.showCurrentEdition) {
        // don't render erasable for previews
        drawErasableStroke(cr, s);
    } else if (s->hasPressure() && !highlighter) {
        drawWithPressure(cr);
    } else {
        drawNoPressure(cr);
    }

    if (useMask) {
        /**
         * Blit the mask onto the target cairo context.
         */

        /**
         * Opacity for the mask's content: the base value depends on the tool:
         * Pen                     : 255
         * Highlighter (no filling): HIGHLIGHTER_ALPHA
         * Highlighter (filled)    : s->getFill()
         */
        double groupAlpha =
                highlighter ? static_cast<double>(filledHighlighter ? s->getFill() : HIGHLIGHTER_ALPHA) : 255.0;

        // If the stroke has no audio attached, we draw it (even more) translucent
        if (drawTranslucent) {
            groupAlpha *= OPACITY_NO_AUDIO;
            groupAlpha = std::max(MINIMAL_ALPHA, groupAlpha);
        }

        // Blit the mask onto the given cairo context
        cairo_set_operator(ctx.cr, highlighter ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER);

        DocumentView::applyColor(ctx.cr, s, (uint8_t)(groupAlpha));

        cairo_mask_surface(ctx.cr, surfMask, 0, 0);

        cairo_destroy(cr);
        cr = nullptr;
        cairo_surface_destroy(surfMask);
        surfMask = nullptr;
    }

    cairo_restore(ctx.cr);
}
