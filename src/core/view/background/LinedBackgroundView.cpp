#include "LinedBackgroundView.h"

#include <memory>  // for allocator

#include "model/BackgroundConfig.h"               // for BackgroundConfig
#include "view/background/BackgroundView.h"       // for view
#include "view/background/RuledBackgroundView.h"  // for RuledBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

LinedBackgroundView::LinedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                         const BackgroundConfig& config):
        RuledBackgroundView(pageWidth, pageHeight, backgroundColor, config) {
    loadVLineColor(config, DEFAULT_V_LINE_COLOR, ALT_DEFAULT_V_LINE_COLOR);
    config.loadValue(CFG_MARGIN, margin);
}

void LinedBackgroundView::draw(cairo_t* cr) const {
    // Paint the horizontal lines
    RuledBackgroundView::draw(cr);

    // Add the vertical line
    drawVerticalMarginLine(cr, margin, lineWidth);
}
