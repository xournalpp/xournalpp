/*
 * Xournal++
 *
 * Displays an ImageFrame Element
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "util/Color.h"  // for Color

#include "View.h"  // for ElementView

class ImageFrame;

namespace xoj::view {

class ImageFrameView: public ElementView {
public:
    explicit ImageFrameView(const ImageFrame* imgFrame);
    ~ImageFrameView() override;

    /**
     * Draws an ImageFrame model to a cairo surface
     */
    void draw(const Context& ctx) const override;

    void drawFrame(cairo_t* cr, Color color) const;
    void drawImage(cairo_t* cr, double alphaForIgnore) const;
    void drawFrameHandles(cairo_t* cr, Color color) const;

    void setZoomForDrawing(double zoom) const;

private:
    const ImageFrame* imageFrame;

    mutable double ZOOM = 1.0;

    Color BLACK = ColorU8{0, 0, 0};
    Color WHITE = ColorU8{255, 255, 255};
};

};  // namespace xoj::view
