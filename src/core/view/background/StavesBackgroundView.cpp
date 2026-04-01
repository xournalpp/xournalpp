#include "StavesBackgroundView.h"

#include "model/BackgroundConfig.h"                  // for BackgroundConfig
#include "view/background/BackgroundView.h"          // for view
#include "view/background/OneColorBackgroundView.h"  // for OneColorBackgrou...
#include "view/background/PlainBackgroundView.h"     // for PlainBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

StavesBackgroundView::StavesBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                           const BackgroundConfig& config):
        OneColorBackgroundView(pageWidth, pageHeight, backgroundColor, config, DEFAULT_LINE_WIDTH, DEFAULT_LINE_COLOR,
                               ALT_DEFAULT_LINE_COLOR) {}

void StavesBackgroundView::draw(cairo_t* cr) const {
    // Paint the background color
    PlainBackgroundView::draw(cr);

    // Get the bounds of the mask, in page coordinates
    double minX;
    double maxX;
    double minY;
    double maxY;
    cairo_clip_extents(cr, &minX, &minY, &maxX, &maxY);

    const double staffHeight = lineWidth + 4 * LINES_SPACING;

    // The `+ 4 * lineWidth` is here as a legacy and it does not make a lot of sense.
    // Still, we must keep it so that we do not alter how staves are displayed in pre-existing documents
    const double vOffsetBetweenStaves = STAVES_SPACING + staffHeight + 4 * lineWidth;

    auto [indexMinY, indexMaxY] = getIndexBounds(minY - HEADER_SIZE - staffHeight, maxY + 0.5 * lineWidth - HEADER_SIZE,
                                                 vOffsetBetweenStaves, 0.0, pageHeight - HEADER_SIZE - FOOTER_SIZE);

    auto drawStaff = [cr, w = this->pageWidth](double offset) {
        for (int i = 0; i < 5; ++i) {
            double y = offset + i * LINES_SPACING;
            cairo_move_to(cr, MARGIN, y);
            cairo_line_to(cr, w - MARGIN, y);
        }
        cairo_line_to(cr, w - MARGIN, offset);
        cairo_move_to(cr, MARGIN, offset);
        cairo_line_to(cr, MARGIN, offset + 4 * LINES_SPACING);
    };

    for (int i = indexMinY; i <= indexMaxY; ++i) { drawStaff(HEADER_SIZE + i * vOffsetBetweenStaves); }

    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, foregroundColor);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_stroke(cr);
    cairo_restore(cr);
}
