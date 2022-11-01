/*
 * Xournal++
 *
 * Class for lined backgrounds (= ruled + vertical margin line)
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "model/BackgroundConfig.h"  // for BackgroundConfig
#include "util/Color.h"              // for Color

#include "RuledBackgroundView.h"  // for RuledBackgroundView

namespace xoj::view {
class LinedBackgroundView: public RuledBackgroundView {
public:
    LinedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~LinedBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    Color vLineColor;
    double margin = 72.0;  // default = 1 inch. Negative values put the margin on the right hand side.

    constexpr static Color DEFAULT_V_LINE_COLOR = Colors::xopp_deeppink;
    constexpr static Color ALT_DEFAULT_V_LINE_COLOR = Colors::xopp_midnightblue;
};
};  // namespace xoj::view
