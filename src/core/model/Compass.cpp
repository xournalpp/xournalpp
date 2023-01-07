#include "Compass.h"

#include <cmath>             // for sin, cos
#include <initializer_list>  // for initializer_list

#include "gui/inputdevices/CompassInputHandler.h"
#include "view/CompassView.h"

Compass::Compass(): Compass(INITIAL_HEIGHT, .0, INITIAL_X, INITIAL_Y) {}

Compass::Compass(double height, double rotation, double x, double y): GeometryTool(height, rotation, x, y) {
    this->lastRepaintRange = getToolRange(true);
}

Compass::~Compass() { viewPool->dispatchAndClear(xoj::view::CompassView::FINALIZATION_REQUEST, lastRepaintRange); }

auto Compass::getToolRange(bool transformed) const -> Range {
    const auto h = height * CM;
    Range rg;
    if (transformed) {
        rg.addPoint(translationX - h, translationY - h);
        rg.addPoint(translationX + h, translationY + h);
    } else {
        rg.addPoint(-h, -h);
        rg.addPoint(h, h);
    }

    // Padding required to fully render the boundary red lines and last blue digit
    constexpr double RENDER_PADDING = 2.0;

    rg.addPadding(RENDER_PADDING + .5 * xoj::view::CompassView::LINE_WIDTH_IN_CM * CM);  // account for line width
    return rg;
}

void Compass::notify(bool resetMask) const {
    if (resetMask) {
        viewPool->dispatch(xoj::view::GeometryToolView::RESET_MASK);
    }

    viewPool->dispatch(xoj::view::CompassView::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                       this->getMatrix());
    Range rg = this->getToolRange(true);
    viewPool->dispatch(xoj::view::CompassView::FLAG_DIRTY_REGION, this->computeRepaintRange(rg));
    handlerPool->dispatch(CompassInputHandler::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                          this->getTranslationX(), this->getTranslationY());
}
