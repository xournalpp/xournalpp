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

    if (backgroundColor.isLight()) {
        vLineColor = getColorOr(config, CFG_FOREGROUND_COLOR_2, DEFAULT_V_LINE_COLOR);
    } else {
        vLineColor = getColorOr(config, CFG_ALT_FOREGROUND_COLOR_2, ALT_DEFAULT_V_LINE_COLOR);
    }

    config.loadValue(CFG_MARGIN, margin);
    if (margin < 0) {
        // A negative value puts the margin line on the right hand side
        margin += pageWidth;
    }
}

void LinedBackgroundView::draw(cairo_t* cr) const {
    // Paint the horizontal lines
    RuledBackgroundView::draw(cr);

    // Add the vertical line
    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, vLineColor);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_move_to(cr, margin, 0);
    cairo_line_to(cr, margin, pageHeight);
    cairo_stroke(cr);
    cairo_restore(cr);
}
