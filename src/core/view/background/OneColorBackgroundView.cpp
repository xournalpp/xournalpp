#include "OneColorBackgroundView.h"

#include <cinttypes>  // for uint32_t

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

Color OneColorBackgroundView::getColorOr(const BackgroundConfig& config, const std::string& str,
                                         const Color& defaultColor) {
    if (uint32_t hexColor; config.loadValueHex(str, hexColor)) {
        return Color(hexColor);
    }
    return defaultColor;
}
