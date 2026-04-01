#include "IsoDottedBackgroundView.h"

#include "model/BackgroundConfig.h"                       // for BackgroundC...
#include "view/background/BackgroundView.h"               // for view
#include "view/background/BaseIsometricBackgroundView.h"  // for BaseIsometr...

using namespace xoj::view;

IsoDottedBackgroundView::IsoDottedBackgroundView(double pageWidth, double pageHeight, Color backgroundColor,
                                                 const BackgroundConfig& config):
        BaseIsometricBackgroundView(pageWidth, pageHeight, backgroundColor, config, DEFAULT_LINE_WIDTH) {}


void IsoDottedBackgroundView::paintGrid(cairo_t* cr, int cols, int rows, double xstep, double ystep, double xOffset,
                                        double yOffset) const {

    auto drawDot = [&](double x, double y) {
        cairo_move_to(cr, xOffset + x, yOffset + y);
        cairo_line_to(cr, xOffset + x, yOffset + y);
    };

    for (int col = 0; col <= cols; ++col) {
        const auto x = col * xstep;

        const auto evenCol = col % 2 == 0;
        const auto yoffset = evenCol ? ystep : 0.0;
        const auto rowsInCol = evenCol ? rows - 2 : rows;

        for (int row = 0; row <= rowsInCol; row += 2) {
            const auto y = yoffset + row * ystep;
            drawDot(x, y);
        }
    }
}
