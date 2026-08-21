#include "ElectronicsHandler.h"

#include <cmath>

ElectronicsHandler::ElectronicsHandler(Control* control, const PageRef& page)
    : BaseShapeHandler(control, page) {
}

ElectronicsHandler::~ElectronicsHandler() = default;

std::pair<std::vector<Point>, Range> ElectronicsHandler::createShape(bool isAltDown, bool isShiftDown, bool isControlDown) {
    std::vector<Point> shape;
    Range range;

    double dx = currPoint.x - startPoint.x;
    double dy = currPoint.y - startPoint.y;

    if (std::abs(dx) < 1.0) dx = 1.0;

    // Amplitude scales with drag distance, capped between sensible boundaries
    double amplitude = std::abs(dy) / 2.0;
    if (amplitude < 10.0) amplitude = 10.0;

    // Dynamically calculate periods. e.g. 1 period per 100 horizontal pixels
    int periods = std::max(1, static_cast<int>(std::abs(dx) / 100.0));

    // Width per period
    double periodWidth = dx / periods;

    // We will generate points for the sine wave along the horizontal drag vector
    int pointsPerPeriod = 40;

    double step = periodWidth / pointsPerPeriod;

    for (int p = 0; p < periods; ++p) {
        for (int i = 0; i <= pointsPerPeriod; ++i) {
            if (p > 0 && i == 0) continue; // avoid duplicate points at period boundaries

            double lx = step * i; // local x within period
            // Sine math:
            double val = std::sin((lx / periodWidth) * 2 * M_PI);

            double px = startPoint.x + (p * periodWidth) + lx;
            double py = startPoint.y - (val * amplitude);

            Point pt(px, py);
            shape.push_back(pt);

            if (p == 0 && i == 0) {
                range = Range(px, py, px, py);
            } else {
                range = range.unite(Range(px, py, px, py));
            }
        }
    }

    return {shape, range};
}
