/*
 * Xournal++
 *
 * Base view for shapes or spline tools
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */
#pragma once

#include <cairo.h>

#include "util/Range.h"
#include "view/Mask.h"

#include "BaseStrokeToolView.h"

class InputHandler;

namespace xoj::view {
class Repaintable;

class BaseShapeOrSplineToolView: public BaseStrokeToolView {

public:
    BaseShapeOrSplineToolView(const InputHandler* handler, Repaintable* parent);
    ~BaseShapeOrSplineToolView() noexcept override;

protected:
    cairo_t* prepareContext(cairo_t* cr) const;

    void commitDrawing(cairo_t* cr) const;

    const double fillingAlpha;

    // The mask is only for filled highlighter strokes, to avoid artefacts as in
    // https://github.com/xournalpp/xournalpp/issues/3709
    mutable Mask mask;
    const bool needMask;
    /// @brief The part of the mask that needs to be wiped to ensure the filling is correctly drawn.
    mutable Range maskWipeExtent;
};
};  // namespace xoj::view
