/*
 * Xournal++
 *
 * Class for graph backgrounds
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
class DottedBackgroundView: public OneColorBackgroundView {
public:
    DottedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~DottedBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    double squareSize = 14.17;  // 5mm

    constexpr static Color DEFAULT_LINE_COLOR = Colors::xopp_silver;
    constexpr static Color ALT_DEFAULT_LINE_COLOR = Colors::xopp_darkslategray;
    constexpr static double DEFAULT_LINE_WIDTH = 1.5;
};
};  // namespace xoj::view
