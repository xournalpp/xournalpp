#include "RuledBackgroundView.h"

#include <memory>  // for allocator

#include "model/BackgroundConfig.h"                  // for BackgroundConfig
#include "view/background/BackgroundView.h"          // for view
#include "view/background/OneColorBackgroundView.h"  // for OneColorBackgrou...
#include "view/background/PlainBackgroundView.h"     // for PlainBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

RuledBackgroundView::RuledBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                         const BackgroundConfig& config):
        OneColorBackgroundView(pageWidth, pageHeight, backgroundColor, config, DEFAULT_LINE_WIDTH, DEFAULT_H_LINE_COLOR,
                               ALT_DEFAULT_H_LINE_COLOR) {
    config.loadValue(CFG_RASTER, lineSpacing);
}

void RuledBackgroundView::draw(cairo_t* cr) const {
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
    auto [indexMinY, indexMaxY] =
            getIndexBounds(minY - HEADER_SIZE - 0.5 * lineWidth, maxY - HEADER_SIZE + 0.5 * lineWidth, lineSpacing, 0.0,
                           pageHeight - HEADER_SIZE - FOOTER_SIZE);

    for (int i = indexMinY; i <= indexMaxY; ++i) {
        cairo_move_to(cr, minX, HEADER_SIZE + i * lineSpacing);
        cairo_line_to(cr, maxX, HEADER_SIZE + i * lineSpacing);
    }

    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, foregroundColor);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_stroke(cr);
    cairo_restore(cr);
}
