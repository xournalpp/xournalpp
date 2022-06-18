#include "DottedBackgroundView.h"

#include <memory>  // for allocator

#include "model/BackgroundConfig.h"                  // for BackgroundConfig
#include "view/background/BackgroundView.h"          // for view
#include "view/background/OneColorBackgroundView.h"  // for OneColorBackgrou...
#include "view/background/PlainBackgroundView.h"     // for PlainBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

DottedBackgroundView::DottedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                           const BackgroundConfig& config):
        OneColorBackgroundView(pageWidth, pageHeight, backgroundColor, config, DEFAULT_LINE_WIDTH, DEFAULT_LINE_COLOR,
                               ALT_DEFAULT_LINE_COLOR) {

    config.loadValue(CFG_RASTER, squareSize);
}

void DottedBackgroundView::draw(cairo_t* cr) const {
    // Paint the background color
    PlainBackgroundView::draw(cr);

    // Get the bounds of the mask, in page coordinates
    double minX;
    double maxX;
    double minY;
    double maxY;
    cairo_clip_extents(cr, &minX, &minY, &maxX, &maxY);

    //  Add a 0.5 * lineWidth padding in case the line is just outside the mask but its thickness still makes it
    //  (partially) visible
    const double halfLineWidth = 0.5 * lineWidth;
    auto [indexMinX, indexMaxX] =
            getIndexBounds(minX - halfLineWidth, maxX + halfLineWidth, squareSize, squareSize, pageWidth);
    auto [indexMinY, indexMaxY] =
            getIndexBounds(minY - halfLineWidth, maxY + halfLineWidth, squareSize, squareSize, pageHeight);

    for (int i = indexMinX; i <= indexMaxX; ++i) {
        double x = i * squareSize;
        for (int j = indexMinY; j <= indexMaxY; ++j) {
            double y = j * squareSize;
            cairo_move_to(cr, x, y);
            cairo_line_to(cr, x, y);
        }
    }

    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, foregroundColor);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_stroke(cr);
    cairo_restore(cr);
}
