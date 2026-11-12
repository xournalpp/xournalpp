/*
 * Xournal++
 *
 * Class for graph backgrounds with a vertical line (= graph + vertical margin line)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "util/Color.h"  // for Color

#include "GraphBackgroundView.h"  // for GraphBackgroundView

class BackgroundConfig;

namespace xoj::view {
class LinedGraphBackgroundView: public GraphBackgroundView {
public:
    LinedGraphBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~LinedGraphBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    double vLinePos = 72.0;  // default = 1 inch. Negative values put the margin on the right hand side.

    constexpr static Color DEFAULT_V_LINE_COLOR = Colors::xopp_deeppink;
    constexpr static Color ALT_DEFAULT_V_LINE_COLOR = Colors::xopp_midnightblue;
};
};  // namespace xoj::view