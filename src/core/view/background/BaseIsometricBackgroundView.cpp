#include "BaseIsometricBackgroundView.h"

#include <cmath>   // for floor, sqrt
#include <memory>  // for allocator

#include "model/BackgroundConfig.h"                  // for BackgroundConfig
#include "view/background/BackgroundView.h"          // for view
#include "view/background/OneColorBackgroundView.h"  // for OneColorBackgrou...
#include "view/background/PlainBackgroundView.h"     // for PlainBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

BaseIsometricBackgroundView::BaseIsometricBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                                         const BackgroundConfig& config, double defaultLineWidth):
        OneColorBackgroundView(pageWidth, pageHeight, backgroundColor, config, defaultLineWidth, DEFAULT_LINE_COLOR,
                               ALT_DEFAULT_LINE_COLOR) {

    config.loadValue(CFG_RASTER, triangleSize);
}

void BaseIsometricBackgroundView::draw(cairo_t* cr) const {
    // Paint the background color
    PlainBackgroundView::draw(cr);

    const double xstep = std::sqrt(3.0) / 2.0 * triangleSize;
    const double ystep = triangleSize / 2.0;

    // Deduce the maximum grid size
    const double margin = triangleSize;
    int cols = static_cast<int>(std::floor((pageWidth - 2 * margin) / xstep));
    int rows = static_cast<int>(std::floor((pageHeight - 2 * margin) / ystep));

    // Center the grid on the page
    const double contentWidth = cols * xstep;
    const double contentHeight = rows * ystep;
    double contentXOffset = (pageWidth - contentWidth) / 2;
    double contentYOffset = (pageHeight - contentHeight) / 2;

    // Get the bounds of the mask, in page coordinates
    double minX;
    double maxX;
    double minY;
    double maxY;
    cairo_clip_extents(cr, &minX, &minY, &maxX, &maxY);

    // Get the indices we need to paint (= on or near the mask)
    const double halfLineWidth = 0.5 * lineWidth;
    auto [indexMinX, indexMaxX] =
            getIndexBounds(minX - halfLineWidth - xstep - contentXOffset, maxX + halfLineWidth + xstep - contentXOffset,
                           xstep, 0.0, contentWidth);
    auto [indexMinY, indexMaxY] =
            getIndexBounds(minY - halfLineWidth - ystep - contentYOffset, maxY + halfLineWidth + ystep - contentYOffset,
                           ystep, 0.0, contentHeight);

    if ((indexMinY + indexMinX) % 2) {
        /** The painted area should start with \/\/\/...
         *                                     /\/\/\/..
         *
         * Enlarge by 1 so that it starts with /\/\/...
         *                                     \/\/\/..
         */
        if (indexMinX) {
            --indexMinX;
        } else {
            --indexMinY;
        }
    }

    // Compute the offset and the number of steps within the mask
    contentXOffset += indexMinX * xstep;
    contentYOffset += indexMinY * ystep;

    cols = indexMaxX - indexMinX;
    rows = indexMaxY - indexMinY;

    paintGrid(cr, cols, rows, xstep, ystep, contentXOffset, contentYOffset);

    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, foregroundColor);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_stroke(cr);
    cairo_restore(cr);
}
