#include "StrokeView.h"

#include <math.h>

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

void StrokeView::drawCalligraphicOnePolygon(double nibAngle, double thickness) {
    // To save calculations in the loop
    double rise = sin(nibAngle), run = cos(nibAngle) * 1;
    // This is what gives the nib it's slant and thickness
    double yShift = rise * thickness;
    double xShift = run * thickness;

    // Setting up for the first iteration
    bool movingUp = false;
    auto path = s->getPointVector();

    int pathLen = path.size();
    g_message("Path length, %d", pathLen);

    bool firstIteration = true;
    int currReversingIndex, nextReversingIndex;
    int backAndForthIterations = (2 * pathLen) - 1;

    // We subtract one because we look ahead one
    for (int i = 0; i < backAndForthIterations - 1; i++) {
        if (i <= pathLen - 2) {
            // Before midway point
            currReversingIndex = i;
            nextReversingIndex = currReversingIndex + 1;
        } else {
            currReversingIndex = (pathLen - 2) - (i - pathLen);
            nextReversingIndex = currReversingIndex - 1;
        }

        Point pCurr = path[currReversingIndex], pNext = path[nextReversingIndex];

        // Move everything to the origin, then check if the next point is above the line
        bool nextIsAbove = rise * (pNext.x - pCurr.x) < run * (pNext.y - pCurr.y);

        // Notice it will be false on first iteration due to the initial values of moving_(up/down)
        // Also, it's value is based on the last iteration's moving_(up/down) value
        bool switchedDirection = nextIsAbove ^ movingUp;

        // Update movement direction for this iteration
        movingUp = nextIsAbove;

        // What if the next point is on the slope?
        /*
         * If you had two points laying on the same slope of then pen, then no matter how you connect the two
         * the result is always identical, so long as you draw a line from one side to another once.
         * Due to this, we don't have to check for if the next point lays on the same line or have any special
         * behavior in this case
         */
        double multiplier = (nextIsAbove) ? 1.0 : -1.0;

        // This section deals with drawing the corners/start/end points of the stroke
        // It also makes sure that the winding number for intersections is exactly +-n != 0 (where n is the # of
        // intersections)
        if (firstIteration) {
            cairo_line_to(cr, pCurr.x - multiplier * (xShift * pCurr.z), pCurr.y - multiplier * (yShift * pCurr.z));
            cairo_line_to(cr, pCurr.x + multiplier * (xShift * pCurr.z), pCurr.y + multiplier * (yShift * pCurr.z));
        } else {
            if (switchedDirection) {
                // The reason why this works, is because if movingUp is true, then
                // previously you were moving down (because you switched), (or vise versa)
                // this makes it so that you draw in the correct direction.
                cairo_line_to(cr, pCurr.x + multiplier * (xShift * pCurr.z), pCurr.y + multiplier * (yShift * pCurr.z));
            }
        }
        // Deals with drawing strokes in a single direction
        cairo_line_to(cr, pNext.x + multiplier * (xShift * pNext.z), pNext.y + multiplier * (yShift * pNext.z));
        firstIteration = false;
    }
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
}


static int count = 0;

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
        // drawNoPressure();
        drawCalligraphicOnePolygon(M_PI / 8, 3.0);
        g_message("Draw calligraphic stroke (without Pressure), no %d!", ++count);
    } else {
        // drawWithPressure();
        drawCalligraphicOnePolygon(M_PI / 8, 3.0);
        g_message("Draw calligraphic stroke (with Pressure), no %d!", ++count);
    }
}
