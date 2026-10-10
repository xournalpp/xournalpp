#include "OneColorBackgroundView.h"

#include <cstdint>  // for uint32_t

#include "model/BackgroundConfig.h"               // for BackgroundConfig
#include "view/background/BackgroundView.h"       // for view
#include "view/background/PlainBackgroundView.h"  // for PlainBackgroundView

using namespace background_config_strings;
using namespace xoj::view;

OneColorBackgroundView::OneColorBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                               const BackgroundConfig& config, double defaultLineWidth,
                                               Color defaultLineColor, Color altDefaultLineColor):
        PlainBackgroundView(pageWidth, pageHeight, backgroundColor), lineWidth(defaultLineWidth) {

    config.loadValue(CFG_LINE_WIDTH, lineWidth);

    if (backgroundColor.isLight()) {
        foregroundColor = getColorOr(config, CFG_FOREGROUND_COLOR_1, defaultLineColor);
    } else {
        foregroundColor = getColorOr(config, CFG_ALT_FOREGROUND_COLOR_1, altDefaultLineColor);
    }
}

void xoj::view::OneColorBackgroundView::multiplyLineWidth(double factor) { lineWidth *= factor; }

void xoj::view::OneColorBackgroundView::loadVLineColor(const BackgroundConfig& config, Color defaultLightColor,
                                                       Color defaultDarkColor) {
    if (backgroundColor.isLight()) {
        vLineColor = getColorOr(config, CFG_FOREGROUND_COLOR_2, defaultLightColor);
    } else {
        vLineColor = getColorOr(config, CFG_ALT_FOREGROUND_COLOR_2, defaultDarkColor);
    }
}

void xoj::view::OneColorBackgroundView::drawVerticalMarginLine(cairo_t* cr, double margin, double lineWidth) const {
    if (margin < 0) {
        // A negative value puts the margin line on the right hand side
        margin += pageWidth;
    }
    cairo_save(cr);
    Util::cairo_set_source_rgbi(cr, vLineColor);
    cairo_set_line_width(cr, lineWidth);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_move_to(cr, margin, 0);
    cairo_line_to(cr, margin, pageHeight);
    cairo_stroke(cr);
    cairo_restore(cr);
}

Color OneColorBackgroundView::getColorOr(const BackgroundConfig& config, const std::string& str,
                                         const Color& defaultColor) {
    if (uint32_t hexColor; config.loadValueHex(str, hexColor)) {
        return Color(hexColor);
    }
    return defaultColor;
}
