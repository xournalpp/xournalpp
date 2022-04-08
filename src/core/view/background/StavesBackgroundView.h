/*
 * Xournal++
 *
 * Class for staves backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "OneColorBackgroundView.h"

class BackgroundConfig;

class xoj::view::StavesBackgroundView: public OneColorBackgroundView {
public:
    StavesBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~StavesBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    constexpr static Color DEFAULT_LINE_COLOR = Color(0x000000U);
    constexpr static Color ALT_DEFAULT_LINE_COLOR = Color(0xffffffU);
    constexpr static double DEFAULT_LINE_WIDTH = 0.5;

    constexpr static double HEADER_SIZE = 80.0;
    constexpr static double FOOTER_SIZE = 60.0;
    constexpr static double MARGIN = 50.0;
    constexpr static double STAVES_SPACING = 40.0;  // Between two staves
    constexpr static double LINES_SPACING = 5.0;    // Between two lines within a staff
};
