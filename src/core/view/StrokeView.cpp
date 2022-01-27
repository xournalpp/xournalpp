#include "StrokeView.h"

#include <cmath>

#include "model/Stroke.h"
#include "model/eraser/ErasableStroke.h"
#include "util/LoopUtil.h"

#include "DocumentView.h"

StrokeView::StrokeView(cairo_t* cr, Stroke* s): cr(cr), crEffective(cr), s(s) {}


void StrokeView::pathToCairo() const {
    for_first_then_each(
            s->getPointVector(), [this](auto const& first) { cairo_move_to(this->crEffective, first.x, first.y); },
            [this](auto const& other) { cairo_line_to(this->crEffective, other.x, other.y); });
}

void StrokeView::drawErasableStroke(cairo_t* cr, Stroke* s) {
    ErasableStroke* e = s->getErasable();
    e->draw(cr);
}

/**
 * No pressure sensitivity, one line is drawn
 */
void StrokeView::drawNoPressure() const {
    cairo_set_line_width(crEffective, s->getWidth());

    const double* dashes = nullptr;
    int dashCount = 0;
    s->getLineStyle().getDashes(dashes, dashCount);
    assert((dashCount == 0 && dashes == nullptr) || (dashCount != 0 && dashes != nullptr));
    cairo_set_dash(crEffective, dashes, dashCount, 0);

    pathToCairo();
    cairo_stroke(crEffective);
}

/**
 * Draw a stroke with pressure, for this multiple lines with different widths needs to be drawn
 */
void StrokeView::drawWithPressure() const {
    double dashOffset = 0;
    const double* dashes = nullptr;
    int dashCount = 0;
    s->getLineStyle().getDashes(dashes, dashCount);
    assert((dashCount == 0 && dashes == nullptr) || (dashCount != 0 && dashes != nullptr));

    for (auto p1i = begin(s->getPointVector()), p2i = std::next(p1i), endi = end(s->getPointVector());
         p1i != endi && p2i != endi; ++p1i, ++p2i) {
        auto width = p1i->z != Point::NO_PRESSURE ? p1i->z : s->getWidth();
        cairo_set_line_width(crEffective, width);
        if (dashes) {
            cairo_set_dash(crEffective, dashes, dashCount, dashOffset);
            dashOffset += p1i->lineLengthTo(*p2i);
        }
        cairo_move_to(crEffective, p1i->x, p1i->y);
        cairo_line_to(crEffective, p2i->x, p2i->y);
        cairo_stroke(crEffective);
    }
}

void StrokeView::paint(bool dontRenderEditingStroke, bool markAudioStroke, bool noColor) const {

    cairo_save(cr);

    const bool highlighter = s->getToolType() == STROKE_TOOL_HIGHLIGHTER;
    const bool filledHighlighter = highlighter && s->getFill() != -1;
    const bool drawTranslucent = markAudioStroke && s->getAudioFilename().empty();
    const bool useMask = (!noColor && filledHighlighter) || drawTranslucent;

    // The mask will be colorblind
    noColor = noColor || useMask;

    cairo_surface_t* surfMask = nullptr;

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
        cairo_get_matrix(cr, &matrix);
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
        crEffective = cairo_create(surfMask);

#ifdef DEBUG_SHOW_MASK
        cairo_set_source_rgba(crEffective, 1, 1, 1, 0.3);
        cairo_paint(crEffective);
#endif
    }

    cairo_set_line_join(crEffective, CAIRO_LINE_JOIN_ROUND);

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
                cairo_set_source_rgba(crEffective, 1, 1, 1, 1);
            } else {
                cairo_set_source_rgba(crEffective, 1, 1, 1, static_cast<double>(fill) / 255.0);
            }
        } else {
            DocumentView::applyColor(crEffective, s, static_cast<uint8_t>(fill));
        }
        cairo_set_operator(crEffective, useMask ? CAIRO_OPERATOR_SOURCE : CAIRO_OPERATOR_OVER);
        pathToCairo();

        cairo_fill(crEffective);
    }

    /**
     * Picking the colors and alpha values, and the operator
     */
    if (noColor) {
        /**
         * No color: painting onto an alpha mask. Colors will be applied upon blitting.
         */
        cairo_set_source_rgba(crEffective, 1, 1, 1, 1);
        cairo_set_operator(crEffective, CAIRO_OPERATOR_SOURCE);
    } else if (highlighter) {
        /**
         * Highlighter without filling.
         */
        DocumentView::applyColor(crEffective, s, HIGHLIGHTER_ALPHA);
        cairo_set_operator(crEffective, CAIRO_OPERATOR_MULTIPLY);
    } else {
        /**
         * Normal pen
         */
        DocumentView::applyColor(crEffective, static_cast<Element*>(s));
        cairo_set_operator(crEffective, CAIRO_OPERATOR_SOURCE);
    }

    if (s->getErasable() && !dontRenderEditingStroke) {
        // don't render erasable for previews
        drawErasableStroke(crEffective, s);
    } else if (s->hasPressure() && !highlighter) {
        drawWithPressure();
    } else {
        drawNoPressure();
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
            groupAlpha *= AudioElement::OPACITY_NO_AUDIO;
            groupAlpha = std::max(MINIMAL_ALPHA, groupAlpha);
        }

        cairo_set_operator(cr, highlighter ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER);

        DocumentView::applyColor(cr, s, (uint8_t)(groupAlpha));

        cairo_mask_surface(cr, surfMask, 0, 0);

        cairo_destroy(crEffective);
        crEffective = cr;
        cairo_surface_destroy(surfMask);
        surfMask = nullptr;
    }

    cairo_restore(cr);
}
