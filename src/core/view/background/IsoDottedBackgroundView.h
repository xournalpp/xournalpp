/*
 * Xournal++
 *
 * Class for isometric graph backgrounds
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <cairo.h>  // for cairo_t

#include "util/Color.h"  // for Color

#include "BaseIsometricBackgroundView.h"  // for BaseIsometricBackgroundView

class BackgroundConfig;

namespace xoj::view {
class IsoDottedBackgroundView: public BaseIsometricBackgroundView {
public:
    IsoDottedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor, const BackgroundConfig& config);
    virtual ~IsoDottedBackgroundView() = default;

protected:
    virtual void paintGrid(cairo_t* cr, int cols, int rows, double xstep, double ystep, double xOffset,
                           double yOffset) const override;

protected:
    constexpr static double DEFAULT_LINE_WIDTH = 1.5;
};
};  // namespace xoj::view
