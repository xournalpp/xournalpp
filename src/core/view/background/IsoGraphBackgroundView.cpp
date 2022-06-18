#include "IsoGraphBackgroundView.h"

#include <cmath>  // for floor

#include "model/BackgroundConfig.h"                       // for BackgroundC...
#include "view/background/BackgroundView.h"               // for view
#include "view/background/BaseIsometricBackgroundView.h"  // for BaseIsometr...

using namespace xoj::view;

IsoGraphBackgroundView::IsoGraphBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                               const BackgroundConfig& config):
        BaseIsometricBackgroundView(pageWidth, pageHeight, backgroundColor, config, DEFAULT_LINE_WIDTH) {}


void IsoGraphBackgroundView::paintGrid(cairo_t* cr, int cols, int rows, double xstep, double ystep, double xOffset,
                                       double yOffset) const {

    auto drawLine = [&](double x1, double y1, double x2, double y2) {
        cairo_move_to(cr, xOffset + x1, yOffset + y1);
        cairo_line_to(cr, xOffset + x2, yOffset + y2);
    };

    const auto contentWidth = cols * xstep;
    const auto contentHeight = rows * ystep;

    // Draw Horizontal line on top and bottom
    // NB: if we're drawing only a portion of the page that does not touch the upper or lower edge, those lines are not
    // at their rightful place. They are also out of the cairo mask's clip, so the following is a no-op.
    drawLine(0.0, 0.0, contentWidth, 0.0);
    drawLine(0.0, contentHeight, contentWidth, contentHeight);

    for (int col = 0; col <= cols; ++col) {
        const auto x = col * xstep;
        drawLine(x, 0.0, x, contentHeight);
    }

    // Determine the number of diagonals to draw
    auto hdiags = static_cast<int>(std::floor(cols / 2));
    auto vdiags = static_cast<int>(std::floor(rows / 2));
    auto diags = hdiags + vdiags;
    auto hcorr = cols - 2 * hdiags;
    auto vcorr = rows - 2 * vdiags;

    // Draw diagonals starting in the top left corner (left-down)
    for (int d = 0; d < diags + hcorr * vcorr; ++d) {
        // Point 1 travels horizontally from top left to top right,
        // then from top right to bottom right.
        double x1 = contentWidth, y1 = 0.0;
        if (d < hdiags) {
            x1 = xstep + d * 2 * xstep;
        } else {
            y1 = ystep + (d - hdiags) * 2 * ystep - hcorr * ystep;
        }

        // Point 2 travels vertically from top left to bottom left,
        // then from bottom left to bottom right.
        double x2 = 0.0, y2 = contentHeight;
        if (d < vdiags) {
            y2 = ystep + d * 2 * ystep;
        } else {
            x2 = xstep + (d - vdiags) * 2 * xstep - vcorr * xstep;
        }

        drawLine(x1, y1, x2, y2);
    }

    // Draw diagonals starting in the top right corner (right-down)
    for (int d = 0; d < diags; ++d) {
        // Point 1 travels horizontally from top right to top left,
        // then from top left to bottom left.
        double x1 = 0.0, y1 = 0.0;
        if (d < hdiags) {
            x1 = contentWidth - (xstep + d * 2 * xstep) - hcorr * xstep;
        } else {
            y1 = ystep + (d - hdiags) * 2 * ystep;
        }

        // Point 2 travels vertically from top right to bottom right,
        // then from top bottom right to bottom left.
        double x2 = contentWidth, y2 = contentHeight;
        if (d < vdiags) {
            y2 = ystep + d * 2 * ystep + hcorr * ystep;
        } else {
            x2 = contentWidth - (xstep + (d - vdiags) * 2 * xstep) + (vcorr - hcorr) * xstep;
        }

        drawLine(x1, y1, x2, y2);
    }
}
