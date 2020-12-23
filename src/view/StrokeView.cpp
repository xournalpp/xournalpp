#include "StrokeView.h"

#include "model/Stroke.h"
#include "model/eraser/EraseableStroke.h"
#include "util/LoopUtil.h"

#include "DocumentView.h"

StrokeView::StrokeView(cairo_t* cr, Stroke* s, int startPoint, double scaleFactor, bool noAlpha):
        cr(cr), s(s), startPoint(startPoint), scaleFactor(scaleFactor), noAlpha(noAlpha) {}


void StrokeView::drawFillStroke() {
    for_first_then_each(
            s->getPointVector(), [this](auto const& first) { cairo_move_to(this->cr, first.x, first.y); },
            [this](auto const& other) { cairo_line_to(this->cr, other.x, other.y); });

    cairo_fill(cr);
}

void StrokeView::applyDashed(double offset) {
    const double* dashes = nullptr;
    int dashCount = 0;
    if (s->getLineStyle().getDashes(dashes, dashCount)) {
        cairo_set_dash(cr, dashes, dashCount, offset);
    } else {
        // Disable dash
        cairo_set_dash(cr, nullptr, 0, 0);
    }
}

void StrokeView::drawEraseableStroke(cairo_t* cr, Stroke* s) {
    EraseableStroke* e = s->getEraseable();
    e->draw(cr);
}

/**
 * Change cairo source, used to draw highlighter transparent,
 * but only if not currently drawing and so on (yes, complicated)
 */
void StrokeView::changeCairoSource(bool markAudioStroke) {
    ///////////////////////////////////////////////////////
    // Fill stroke
    ///////////////////////////////////////////////////////
    if (s->getFill() != -1 && s->getToolType() != STROKE_TOOL_HIGHLIGHTER) {
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

        // Set the color and opacity
        DocumentView::applyColor(cr, s, s->getFill());

        drawFillStroke();
    }


    if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER || (s->getAudioFilename().length() == 0 && markAudioStroke)) {
        if (s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
            cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
        } else {
            cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        }

        // Set the color
        DocumentView::applyColor(cr, s, 120);
    } else {
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        // Set the color
        DocumentView::applyColor(cr, s);
    }
}

/**
 * No pressure sensitivity, one line is drawn
 */
void StrokeView::drawNoPressure() {
    double width = s->getWidth();

    bool group = false;
    if (s->getFill() != -1 && s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
        cairo_push_group(cr);
        // Do not apply the alpha here, else the border and the fill
        // are visible instead of one homogeneous area
        DocumentView::applyColor(cr, s, 255);
        drawFillStroke();
        group = true;
    }

    // Set width
    cairo_set_line_width(cr, width * scaleFactor);
    applyDashed(0);

    for_first_then_each(
            s->getPointVector(), [this](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [this](auto const& other) { cairo_line_to(cr, other.x, other.y); });
    cairo_stroke(cr);

    if (group) {
        cairo_pop_group_to_source(cr);

        if (noAlpha) {
            // Currently drawing -> transparent applied on blitting
            cairo_paint(cr);
        } else {
            cairo_paint_with_alpha(cr, s->getFill() / 255.0);
        }
    }
}

/**
 * Draw a stroke with pressure, for this multiple
 * lines with different widths needs to be drawn
 */
void StrokeView::drawWithPressure() {
    double dashOffset = 0;

    for (auto p1i = begin(s->getPointVector()), p2i = std::next(p1i), endi = end(s->getPointVector());
         p1i != endi && p2i != endi; ++p1i, ++p2i) {
        auto width = p1i->z != Point::NO_PRESSURE ? p1i->z : s->getWidth();
        cairo_set_line_width(cr, width * scaleFactor);
        applyDashed(dashOffset);
        cairo_move_to(cr, p1i->x, p1i->y);
        cairo_line_to(cr, p2i->x, p2i->y);
        cairo_stroke(cr);
        dashOffset += p1i->lineLengthTo(*p2i);
    }
}

void StrokeView::paint(bool dontRenderEditingStroke) {
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

    // don't render eraseable for previews
    if (s->getEraseable() && !dontRenderEditingStroke) {
        drawEraseableStroke(cr, s);
        return;
    }

    // No pressure sensitivity, easy draw a line...
    if (!s->hasPressure() || s->getToolType() == STROKE_TOOL_HIGHLIGHTER) {
        drawNoPressure();
    } else {
        drawWithPressure();
    }
}
