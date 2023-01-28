/*
 * Xournal++
 *
 * Draw stroke
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <utility>  // for pair

#include <cairo.h>  // for cairo_t

class ErasableStroke;
class Range;

namespace xoj {
namespace view {
class Mask;

class ErasableStrokeView {
public:
    ErasableStrokeView(const ErasableStroke& erasableStroke);
    ~ErasableStrokeView() = default;

    /**
     * @brief Draw the erasable stroke assuming the cairo context is all set
     * @param cr The cairo context to draw to
     */
    void draw(cairo_t* cr) const;

    /**
     * @brief Draw the filling of the erasable stroke  assuming the cairo context is all set
     * @param cr The cairo context
     */
    void drawFilling(cairo_t* cr) const;

    /**
     * @brief Paint the erasable stroke, after setting up the cairo context, assuming the stroke is filled and made with
     * the highlighter tool
     * @param cr The cairo context
     */
    void paintFilledHighlighter(cairo_t* cr) const;

private:
    /**
     * @brief Create a cairo mask for a given rectangle and sets up a context for drawing on it
     * @param target A cairo context on which the mask would be used
     * @param box The range delimiting the mask
     * @param zoom A zoom ratio to apply to the mask
     */
    xoj::view::Mask createMask(cairo_t* target, const Range& box, double zoom) const;

private:
    const ErasableStroke& erasableStroke;
};
};  // namespace view
};  // namespace xoj
