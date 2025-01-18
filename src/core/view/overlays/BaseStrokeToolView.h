/*
 * Xournal++
 *
 * Virtual class for showing overlays (e.g. active tools, selections and so on)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>

#include "model/LineStyle.h"
#include "util/Color.h"
#include "view/overlays/OverlayView.h"

class Stroke;

namespace xoj::view {
class Mask;
class Repaintable;

class BaseStrokeToolView: public ToolView {
protected:
    BaseStrokeToolView(Repaintable* parent, const Stroke& stroke);
    ~BaseStrokeToolView() noexcept override;

    /**
     * @brief Creates a mask corresponding to the parent's visible area.
     * The mask is optimized (?) to be blitted to a surface of the same type as cairo_get_target(targetCr).
     * If targetCr == nullptr, the surface is of CAIRO_SURFACE_TYPE_IMAGE.
     */
    Mask createMask(cairo_t* targetCr) const;

    /**
     * @brief Helper function to get a color whose alpha value depends on the tool's properties
     */
    static Color strokeColorWithAlpha(const Stroke& s);

    /**
     * @brief All that's required to draw a stroke in cairo
     */
    const cairo_operator_t cairoOp;
    const Color strokeColor;
    const LineStyle lineStyle;
    double strokeWidth;
};
};  // namespace xoj::view
