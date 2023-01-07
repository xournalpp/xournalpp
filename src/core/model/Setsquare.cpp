#include "Setsquare.h"

#include <cmath>             // for sin, cos
#include <initializer_list>  // for initializer_list

#include "gui/inputdevices/SetsquareInputHandler.h"
#include "view/SetsquareView.h"

Setsquare::Setsquare(): Setsquare(INITIAL_HEIGHT, .0, INITIAL_X, INITIAL_Y) {}

Setsquare::Setsquare(double height, double rotation, double x, double y): GeometryTool(height, rotation, x, y) {
    this->lastRepaintRange = getToolRange(true);
}

Setsquare::~Setsquare() {
    viewPool->dispatchAndClear(xoj::view::SetsquareView::FINALIZATION_REQUEST, lastRepaintRange);
}

auto Setsquare::getToolRange(bool transformed) const -> Range {
    const auto h = height * CM;
    Range rg;
    if (transformed) {
        const auto cs = std::cos(rotation);
        const auto si = std::sin(rotation);
        rg.addPoint(translationX + cs * h, translationY - si * h);
        rg.addPoint(translationX - cs * h, translationY + si * h);
        rg.addPoint(translationX - si * h, translationY + cs * h);
    } else {
        rg.addPoint(h, 0);
        rg.addPoint(-h, 0);
        rg.addPoint(0, h);
    }

    // Padding required to fully render the boundary red lines
    constexpr double RENDER_PADDING = 1.0;

    rg.addPadding(RENDER_PADDING + .5 * xoj::view::SetsquareView::LINE_WIDTH_IN_CM * CM);  // account for line width
    return rg;
}

void Setsquare::notify(bool resetMask) const {
    if (resetMask) {
        viewPool->dispatch(xoj::view::GeometryToolView::RESET_MASK);
    }
    viewPool->dispatch(xoj::view::SetsquareView::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                       this->getMatrix());
    Range rg = this->getToolRange(true);
    viewPool->dispatch(xoj::view::SetsquareView::FLAG_DIRTY_REGION, this->computeRepaintRange(rg));
    handlerPool->dispatch(SetsquareInputHandler::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                          this->getTranslationX(), this->getTranslationY());
}
