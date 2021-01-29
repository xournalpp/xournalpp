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
            cairo_line_to(cr, pCurr.x - multiplier * (xShift * std::abs(pCurr.z)),
                          pCurr.y - multiplier * (yShift * std::abs(pCurr.z)));
            cairo_line_to(cr, pCurr.x + multiplier * (xShift * std::abs(pCurr.z)),
                          pCurr.y + multiplier * (yShift * std::abs(pCurr.z)));
        } else {
            if (switchedDirection) {
                // The reason why this works, is because if movingUp is true, then
                // previously you were moving down (because you switched), (or vise versa)
                // this makes it so that you draw in the correct direction.
                cairo_line_to(cr, pCurr.x + multiplier * (xShift * std::abs(pCurr.z)),
                              pCurr.y + multiplier * (yShift * std::abs(pCurr.z)));
            }
        }
        // Deals with drawing strokes in a single direction
        cairo_line_to(cr, pNext.x + multiplier * (xShift * std::abs(pNext.z)),
                      pNext.y + multiplier * (yShift * std::abs(pNext.z)));
        firstIteration = false;
    }
    cairo_close_path(cr);
    cairo_fill_preserve(cr);
}

void StrokeView::draw_path_calligraphic(cairo_path_t* path, double angle, double thickness, bool fill) {
    double last_move_x = 0., last_move_y = 0.;
    double current_point_x = 0., current_point_y = 0.;
    double x_shift = cos(angle) * thickness;
    double y_shift = sin(angle) * thickness;

    // Go through the path.  For each path segment, we draw a small rectangle.
    for (int i = 0; i < path->num_data; i += path->data[i].header.length) {
        cairo_path_data_t* data = &path->data[i];
        double x, y;
        switch (data->header.type) {
            case CAIRO_PATH_MOVE_TO:
                last_move_x = data[1].point.x;
                last_move_y = data[1].point.y;
                current_point_x = data[1].point.x;
                current_point_y = data[1].point.y;
                break;
            case CAIRO_PATH_LINE_TO:
            case CAIRO_PATH_CLOSE_PATH:
                if (data->header.type == CAIRO_PATH_LINE_TO) {
                    x = data[1].point.x;
                    y = data[1].point.y;
                } else {
                    x = last_move_x;
                    y = last_move_y;
                }
                // printf("%g,%g to %g,%g\n", current_point_x, current_point_y, x, y);
                cairo_move_to(cr, current_point_x + x_shift, current_point_y + y_shift);
                cairo_line_to(cr, current_point_x - x_shift, current_point_y - y_shift);
                cairo_line_to(cr, x - x_shift, y - y_shift);
                cairo_line_to(cr, x + x_shift, y + y_shift);
                cairo_close_path(cr);
                if (fill)
                    cairo_fill(cr);
                else
                    cairo_stroke(cr);

                current_point_x = x;
                current_point_y = y;
                break;
            case CAIRO_PATH_CURVE_TO:
                assert(0 && "curve to should not be present since we use cairo_copy_path_flat");
                break;
            default:
                assert(0 && "Unknown path command");
        }
    }
}

void StrokeView::stroke_calligraphic(double angle, double thickness) {
    cairo_pattern_t* mask;
    cairo_path_t* path;

    // Get the current path. This uses _flat so that cairo replaces
    // cairo_curve_to() calls with many line_to()s.
    path = cairo_copy_path_flat(cr);
    cairo_new_path(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    // I get antialiasing artifacts with OVER where two of the rectangles meet. ADD seems to make that problem go away.
    cairo_set_operator(cr, CAIRO_OPERATOR_ADD);

    // Redirect drawing to a temporary surface that we use to prepare the path.
    // This surface only has an alpha channel. It starts all transparent. The code
    // below draws to it, making it opaque where needed.
    cairo_push_group_with_content(cr, CAIRO_CONTENT_ALPHA);

    draw_path_calligraphic(path, angle, thickness, true);

    // Now draw through the mask.
    // We used cairo_save() above. The cairo_restore() here now restores the
    // source that was set by the caller (e.g. through cairo_set_source_rgb).
    mask = cairo_pop_group(cr);
    cairo_restore(cr);
    cairo_mask(cr, mask);

    cairo_pattern_destroy(mask);
    cairo_path_destroy(path);
}

void StrokeView::outline_calligraphic(double angle, double thickness) {
    cairo_pattern_t* mask;
    cairo_path_t* path;

    // Get the current path. This uses _flat so that cairo replaces
    // cairo_curve_to() calls with many line_to()s.
    path = cairo_copy_path_flat(cr);
    cairo_new_path(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    // I get antialiasing artifacts with OVER where two of the rectangles meet. ADD seems to make that problem go away.
    cairo_set_operator(cr, CAIRO_OPERATOR_ADD);

    // Redirect drawing to a temporary surface that we use to prepare the path.
    // This surface only has an alpha channel. It starts all transparent. The code
    // below draws to it, making it opaque where needed.
    cairo_push_group_with_content(cr, CAIRO_CONTENT_ALPHA);

    // Draw the outline (false -> stroke). This also draws some "interior lines" in
    // the middle of the shape.
    draw_path_calligraphic(path, angle, thickness, false);

    // Fill the middle of the shape with transparency, removing these artifacts.
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    draw_path_calligraphic(path, angle, thickness, true);

    // Now draw through the mask.
    // We used cairo_save() above. The cairo_restore() here now restores the
    // source that was set by the caller (e.g. through cairo_set_source_rgb).
    mask = cairo_pop_group(cr);
    cairo_restore(cr);
    cairo_mask(cr, mask);

    cairo_pattern_destroy(mask);
    cairo_path_destroy(path);
}

void StrokeView::draw_calligraphic(double angle, double thickness) {

#if 0
    for_first_then_each(
            s->getPointVector(), [this](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [this](auto const& other) { cairo_line_to(cr, other.x, other.y); });

    cairo_set_line_width(cr, 4);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_BEVEL);
    cairo_set_source_rgb(cr, 0, 1, 0);
    outline_calligraphic(angle, thickness);
#endif

    cairo_pattern_t* pattern = cairo_pattern_create_linear(0, 0, 800, 800);
    cairo_pattern_add_color_stop_rgba(pattern, 0, 1, 0, 0, 0.75);
    cairo_pattern_add_color_stop_rgba(pattern, 1, 0, 0, 1, 1);
    cairo_set_source(cr, pattern);
    cairo_pattern_destroy(pattern);

    for_first_then_each(
            s->getPointVector(), [this](auto const& first) { cairo_move_to(cr, first.x, first.y); },
            [this](auto const& other) { cairo_line_to(cr, other.x, other.y); });

    // Now fill the path calligraphically with the current parameters (line width
    // and color/source)
    stroke_calligraphic(angle, thickness);
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
        // drawCalligraphicOnePolygon(M_PI / 8, 1.5);
        draw_calligraphic(M_PI / 8, 3.0);
        // drawNoPressure();
        g_message("Draw calligraphic stroke (without Pressure), no %d!", ++count);
    } else {
        // drawCalligraphicOnePolygon(M_PI / 8, 1.5);
        draw_calligraphic(M_PI / 8, 3.0);
        // drawWithPressure();
        g_message("Draw calligraphic stroke (with Pressure), no %d!", ++count);
    }
}
