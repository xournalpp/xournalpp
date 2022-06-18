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

#include <cairo.h>  // for cairo_t, cairo_surface_t

class ErasableStroke;
namespace xoj::util {
template <class T>
class Rectangle;
}  // namespace xoj::util

namespace xoj {
namespace view {
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
     * @param box The rectangle delimiting the mask
     * @param scaling A scaling ratio to apply to the mask
     * @return Pointers to the newly created cairo context and surface (mask)
     */
    std::pair<cairo_t*, cairo_surface_t*> createMask(const xoj::util::Rectangle<double>& box, double scaling) const;

private:
    const ErasableStroke& erasableStroke;
};
};  // namespace view
};  // namespace xoj
