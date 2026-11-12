#include "LinedGraphBackgroundView.h"

#include <cmath>    // for round
#include <memory>   // for allocator

#include "model/BackgroundConfig.h"              // for BackgroundConfig
#include "view/background/GraphBackgroundView.h"  // for GraphBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

LinedGraphBackgroundView::LinedGraphBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                                   const BackgroundConfig& config):
        GraphBackgroundView(pageWidth, pageHeight, backgroundColor, config) {

    // Keep the grid unclamped (the whole page) - only the vertical line is positioned by the margin
    margin = 0.0;

    vLinePos = 72.0;  // default = 1 inch. Negative values put the margin on the right hand side.
    config.loadValue(CFG_MARGIN, vLinePos);
    if (vLinePos < 0) {
        // A negative value puts the margin line on the right hand side
        vLinePos += pageWidth;
    }
    // Snap the line on the nearest grid line so it overlaps an existing grid line
    vLinePos = std::round(vLinePos / squareSize) * squareSize;

    loadVLineColor(config, DEFAULT_V_LINE_COLOR, ALT_DEFAULT_V_LINE_COLOR);
}

void LinedGraphBackgroundView::draw(cairo_t* cr) const {
    // Paint the graph grid
    GraphBackgroundView::draw(cr);

    // Add the vertical line
    drawVerticalMarginLine(cr, vLinePos, lineWidth);
}