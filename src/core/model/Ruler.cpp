#include "Ruler.h"

#include <algorithm>
#include <cmath>

#include "view/RulerView.h"

Ruler::Ruler(): Ruler(INITIAL_LENGTH, 0., 0., 0.) {}

Ruler::Ruler(double length, double rotation, double x, double y): GeometryTool(length, rotation, x, y) {
    lastRepaintRange = getToolRange(true);
}

Ruler::~Ruler() { viewPool->dispatchAndClear(xoj::view::RulerView::FINALIZATION_REQUEST, lastRepaintRange); }

auto Ruler::getToolRange(bool transformed) const -> Range {
    const double halfLength = height * CM / 2.;
    const double thickness = THICKNESS * CM;
    Range rg;
    if (transformed) {
        const double cs = std::cos(rotation);
        const double sn = std::sin(rotation);
        for (const auto& p: {xoj::util::Point<double>(-halfLength, 0.),
                             xoj::util::Point<double>(halfLength, 0.),
                             xoj::util::Point<double>(-halfLength, thickness),
                             xoj::util::Point<double>(halfLength, thickness)}) {
            rg.addPoint(origin.x + cs * p.x - sn * p.y, origin.y + sn * p.x + cs * p.y);
        }
    } else {
        rg.addPoint(-halfLength, 0.);
        rg.addPoint(halfLength, thickness);
    }
    rg.addPadding(2.0);
    return rg;
}

void Ruler::notify(bool resetMask) const {
    if (resetMask) {
        viewPool->dispatch(xoj::view::GeometryToolView::RESET_MASK);
    }
    viewPool->dispatch(xoj::view::RulerView::UPDATE_VALUES, height, rotation, getMatrix());
    viewPool->dispatch(xoj::view::RulerView::FLAG_DIRTY_REGION, computeRepaintRange(getToolRange(true)));
}
