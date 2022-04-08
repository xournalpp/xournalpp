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

#include "RuledBackgroundView.h"

class xoj::view::LinedBackgroundView: public RuledBackgroundView {
public:
    LinedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~LinedBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    Color vLineColor;

    constexpr static Color DEFAULT_V_LINE_COLOR = Color(0xFF0080U);
    constexpr static Color ALT_DEFAULT_V_LINE_COLOR = Color(0x220080U);

    constexpr static double MARGIN_SIZE = 72.0;
};
