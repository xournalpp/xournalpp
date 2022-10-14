#include "StrokeView.h"

#include <algorithm>  // for max
#include <cassert>    // for assert
#include <cmath>      // for ceil

#include <glib.h>  // for g_warning

#include "model/Stroke.h"     // for Stroke, StrokeTool::HIGHLIGHTER
#include "util/Color.h"       // for cairo_set_source_rgbi
#include "util/Rectangle.h"   // for Rectangle
#include "view/View.h"        // for Context, OPACITY_NO_AUDIO, view

#include "ErasableStrokeView.h"  // for ErasableStrokeView
#include "StrokeViewHelper.h"
#include "config-debug.h"        // for DEBUG_SHOW_MASK
#include "filesystem.h"          // for path

class ErasableStroke;

using xoj::util::Rectangle;
using namespace xoj::view;

StrokeView::StrokeView(const Stroke* s): s(s) {}

void StrokeView::draw(const Context& ctx) const {

    if (s->getPointCount() < 2) {
        // Should not happen
        g_warning("View::StrokeView::draw empty stroke...");
        return;
    }

    const bool highlighter = s->getToolType() == StrokeTool::HIGHLIGHTER;
    const bool filledHighlighter = highlighter && s->getFill() != -1;
    const bool drawTranslucent = ctx.fadeOutNonAudio && s->getAudioFilename().empty();
    const bool useMask = (!ctx.noColor && filledHighlighter) || drawTranslucent;

    if (ctx.showCurrentEdition && filledHighlighter && s->getErasable() != nullptr) {
        // Currently being erased filled highlighter strokes need a special treatment
        ErasableStrokeView erasableStrokeView(*s->getErasable());
        erasableStrokeView.paintFilledHighlighter(ctx.cr);
        return;
    }

    // The mask will be colorblind
    const bool noColor = ctx.noColor || useMask;

    cairo_save(ctx.cr);

    cairo_surface_t* surfMask = nullptr;

    // If not using a mask, draw directly onto the given cairo context
    cairo_t* cr = ctx.cr;

    if (useMask) {
        /**
         * To avoid visual glitches when different translucent cairo_stroke are painted,
         * they are painted without colors to a mask which will in turn be blitted (see below)
         */

        /**
         * The mask needs to have to right resolution: the number of pixels per page coordinate units.
         *
         * This value can be recovered from the given canvas: the mask must have the exact same resolution as the canvas
         * This resolution combines both the surface scale and the transformation matrix zoom ratio.
         */
        double scaleX;
        double scaleY;
        cairo_surface_get_device_scale(cairo_get_target(ctx.cr), &scaleX, &scaleY);

        {  // Multiply the scale using the transformation matrix
            cairo_matrix_t matrix;
            cairo_get_matrix(ctx.cr, &matrix);
            // We assume the matrix is diagonal (i.e. only scaling, no rotation)
            assert(matrix.xy == 0 && matrix.yx == 0);

            scaleX *= matrix.xx;
            scaleY *= matrix.yy;
        }

        /**
         * Create a surface tailored to the stroke's bounding box
         */
        Rectangle<double> box = s->boundingRect();

        // Use integral offsets to avoid unnecessary antialiasing upon blitting the mask
        const double offsetX = -std::floor(box.x * scaleX);
        const double offsetY = -std::floor(box.y * scaleY);

        const int width = static_cast<int>(std::ceil((box.x + box.width) * scaleX) + offsetX);
        const int height = static_cast<int>(std::ceil((box.y + box.height) * scaleY) + offsetY);

        surfMask = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);

        // Apply offset and scaling
        cairo_surface_set_device_offset(surfMask, offsetX, offsetY);
        cairo_surface_set_device_scale(surfMask, scaleX, scaleY);

        // Get a context to draw on our mask
        cr = cairo_create(surfMask);

#ifdef DEBUG_SHOW_MASK
        cairo_set_source_rgba(cr, 1, 1, 1, 0.3);
        cairo_paint(cr);
#endif
    }

    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP[s->getStrokeCapStyle()]);

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
            Util::cairo_set_source_rgbi(cr, s->getColor(), static_cast<double>(fill) / 255.0);
        }
        cairo_set_operator(cr, useMask ? CAIRO_OPERATOR_SOURCE : CAIRO_OPERATOR_OVER);

        if (ErasableStroke* erasable = s->getErasable(); erasable != nullptr && ctx.showCurrentEdition) {
            // don't render erasable for previews
            ErasableStrokeView erasableStrokeView(*erasable);
            erasableStrokeView.drawFilling(cr);
        } else {
            StrokeViewHelper::pathToCairo(cr, s->getPointVector());
            cairo_fill(cr);
        }
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
        Util::cairo_set_source_rgbi(cr, s->getColor(), OPACITY_HIGHLIGHTER);
        cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY);
    } else {
        /**
         * Normal pen
         */
        Util::cairo_set_source_rgbi(cr, s->getColor());
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    }

    if (ErasableStroke* erasable = s->getErasable(); erasable != nullptr && ctx.showCurrentEdition) {
        // don't render erasable for previews
        ErasableStrokeView erasableStrokeView(*erasable);
        erasableStrokeView.draw(cr);
    } else if (s->hasPressure() && !highlighter) {
        StrokeViewHelper::drawWithPressure(cr, s->getPointVector(), s->getLineStyle());
    } else {
        StrokeViewHelper::drawNoPressure(cr, s->getPointVector(), s->getWidth(), s->getLineStyle());
    }

    if (useMask) {
        /**
         * Blit the mask onto the target cairo context.
         */

        /**
         * Opacity for the mask's content: the base value depends on the tool:
         * Pen                     : 1
         * Highlighter (no filling): OPACITY_HIGHLIGHTER
         * Highlighter (filled)    : s->getFill() / 255
         */
        double groupAlpha =
                highlighter ? (filledHighlighter ? static_cast<double>(s->getFill()) / 255.0 : OPACITY_HIGHLIGHTER) :
                              1.0;

        // If the stroke has no audio attached, we draw it (even more) translucent
        if (drawTranslucent) {
            groupAlpha *= OPACITY_NO_AUDIO;
            groupAlpha = std::max(MINIMAL_ALPHA, groupAlpha);
        }

        // Blit the mask onto the given cairo context
        cairo_set_operator(ctx.cr, highlighter ? CAIRO_OPERATOR_MULTIPLY : CAIRO_OPERATOR_OVER);

        Util::cairo_set_source_rgbi(ctx.cr, s->getColor(), groupAlpha);

        cairo_mask_surface(ctx.cr, surfMask, 0, 0);

        cairo_destroy(cr);
        cr = nullptr;
        cairo_surface_destroy(surfMask);
        surfMask = nullptr;
    }

    cairo_restore(ctx.cr);
}
