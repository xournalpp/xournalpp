#include "BaseShapeOrSplineToolView.h"

#include "control/tools/InputHandler.h"
#include "model/LineStyle.h"
#include "model/Stroke.h"
#include "util/Assert.h"
#include "util/Color.h"
#include "util/Util.h"  // for cairo_set_dash_from_vector

using namespace xoj::view;

/**
 * @brief To avoid rendering artefact (due to antialiasing?) we add a padding when clearing out the mask (for filled
 * highlighter only)
 */
constexpr double MASK_CLEARANCE_PADDING = 1;

static const Stroke& safeGetStroke(const InputHandler* h) {
    xoj_assert(h);
    xoj_assert(h->getStroke());
    return *h->getStroke();
}

static double getFillingAlpha(const InputHandler* h) {
    auto f = safeGetStroke(h).getFill();
    return f == -1 ? 0.0 : f / 255.0;
}

BaseShapeOrSplineToolView::BaseShapeOrSplineToolView(const InputHandler* toolHandler, Repaintable* parent):
        BaseStrokeToolView(parent, safeGetStroke(toolHandler)),
        fillingAlpha(getFillingAlpha(toolHandler)),
        needMask(this->fillingAlpha != 0.0 && safeGetStroke(toolHandler).getToolType() == StrokeTool::HIGHLIGHTER) {}

BaseShapeOrSplineToolView::~BaseShapeOrSplineToolView() noexcept = default;

cairo_t* BaseShapeOrSplineToolView::prepareContext(cairo_t* cr) const {
    cairo_set_operator(cr, this->cairoOp);

    if (needMask) {
        // The mask is only for filled highlighter strokes, to avoid artefacts as in
        // https://github.com/xournalpp/xournalpp/issues/3709
        if (!mask.isInitialized()) {
            mask = createMask(cr);
            const auto& dashes = this->lineStyle.getDashes();
            Util::cairo_set_dash_from_vector(mask.get(), dashes, 0);
            cairo_set_line_width(mask.get(), this->strokeWidth);
            // operator is already set by createMask().
        } else {
            // Clear the mask
            maskWipeExtent.addPadding(MASK_CLEARANCE_PADDING);
            mask.wipeRange(maskWipeExtent);
            maskWipeExtent = Range();
        }
        return mask.get();
    } else {
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_set_line_width(cr, this->strokeWidth);

        const auto& dashes = this->lineStyle.getDashes();
        Util::cairo_set_dash_from_vector(cr, dashes, 0.0);
        return cr;
    }
}

void BaseShapeOrSplineToolView::commitDrawing(cairo_t* cr) const {
    if (fillingAlpha != 0.0) {
        if (mask.isInitialized()) {
            cairo_fill_preserve(mask.get());
        } else {
            // Not need when using a mask: transparency will be applied upon blitting.
            Util::cairo_set_source_rgbi(cr, strokeColor, fillingAlpha);
            cairo_fill_preserve(cr);
        }
    }

    Util::cairo_set_source_argb(cr, this->strokeColor);

    if (mask.isInitialized()) {
        cairo_stroke(mask.get());
        mask.blitTo(cr);
    } else {
        cairo_stroke(cr);
    }
}
