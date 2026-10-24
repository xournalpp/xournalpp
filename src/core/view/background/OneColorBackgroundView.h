/*
 * Xournal++
 *
 * Class for backgrounds with lineWidth and a line color
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <string>  // for string

#include <cairo.h>  // for cairo_t

#include "util/Color.h"  // for Color

#include "PlainBackgroundView.h"  // for PlainBackgroundView

class BackgroundConfig;

namespace xoj::view {
class OneColorBackgroundView: public PlainBackgroundView {
public:
    OneColorBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config,
                           double defaultLineWidth, Color defaultLineColor, Color altDefaultLineColor);
    virtual ~OneColorBackgroundView() = default;

    void multiplyLineWidth(double factor);

protected:
    /**
     * @brief Get the config's Hex value associated to the config string and make it a color.
     * Fallback to defaultColor if the value does not exist in config.
     */
    static Color getColorOr(const BackgroundConfig& config, const std::string& str, const Color& defaultColor);

    /**
     * @brief Load the color of a vertical margin line from the config (keys "f2"/"af2").
     */
    void loadVLineColor(const BackgroundConfig& config, Color defaultLightColor, Color defaultDarkColor);

    /**
     * @brief Draw a vertical margin line at x=margin, from the top to the bottom of the page.
     * A negative margin puts the line on the right hand side. The line is drawn with the color
     * previously set via loadVLineColor.
     */
    void drawVerticalMarginLine(cairo_t* cr, double margin, double lineWidth) const;

protected:
    Color foregroundColor;
    Color vLineColor;
    double lineWidth;
};
};  // namespace xoj::view
