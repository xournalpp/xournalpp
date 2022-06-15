/*
 * Xournal++
 *
 * Base class for isometric backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include "OneColorBackgroundView.h"

class BackgroundConfig;

namespace xoj::view {
class BaseIsometricBackgroundView: public OneColorBackgroundView {
public:
    BaseIsometricBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                const BackgroundConfig& config, double defaultLineWidth);
    virtual ~BaseIsometricBackgroundView() = default;

    virtual void draw(cairo_t* cr) const override;

protected:
    virtual void paintGrid(cairo_t* cr, int cols, int rows, double xstep, double ystep, double xOffset,
                           double yOffset) const = 0;

protected:
    double triangleSize = 14.17;  // 5mm

    constexpr static Color DEFAULT_LINE_COLOR = Color(0xBDBDBDU);
    constexpr static Color ALT_DEFAULT_LINE_COLOR = Color(0x434343U);
};
};  // namespace xoj::view
