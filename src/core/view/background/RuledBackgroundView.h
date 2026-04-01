/*
 * Xournal++
 *
 * Class for ruled backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "util/Color.h"  // for Color

#include "OneColorBackgroundView.h"  // for OneColorBackgroundView

class BackgroundConfig;

namespace xoj::view {
class RuledBackgroundView: public OneColorBackgroundView {
public:
    RuledBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~RuledBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    double lineSpacing = 24.0;  // Between two horizontal lines

    constexpr static Color DEFAULT_H_LINE_COLOR = Colors::xopp_dodgerblue;
    constexpr static Color ALT_DEFAULT_H_LINE_COLOR = Colors::xopp_darkslategray;
    constexpr static double DEFAULT_LINE_WIDTH = 0.5;

    constexpr static double HEADER_SIZE = 80.0;
    constexpr static double FOOTER_SIZE = 60.0;
};
};  // namespace xoj::view
