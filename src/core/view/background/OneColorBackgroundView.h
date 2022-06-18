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

protected:
    Color foregroundColor;
    double lineWidth;
};
};  // namespace xoj::view
