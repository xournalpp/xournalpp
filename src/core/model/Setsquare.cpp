#include "Setsquare.h"

#include <cmath>             // for sin, cos
#include <initializer_list>  // for initializer_list

#include "gui/inputdevices/SetsquareInputHandler.h"
#include "view/SetsquareView.h"

Setsquare::Setsquare(): Setsquare(INITIAL_HEIGHT, .0, INITIAL_X, INITIAL_Y) {}

Setsquare::Setsquare(double height, double rotation, double x, double y): GeometryTool(height, rotation, x, y) {}

Setsquare::~Setsquare() { viewPool->dispatchAndClear(xoj::view::SetsquareView::FINALIZATION_REQUEST, Range()); }

void Setsquare::notify() const {
    double cs = std::cos(rotation);
    double si = std::sin(rotation);
    double h = height * CM;
    Range rg;
    rg.addPoint(translationX + cs * h, translationY - si * h);
    rg.addPoint(translationX - cs * h, translationY + si * h);
    rg.addPoint(translationX - si * h, translationY + cs * h);
    if (this->stroke) {
        rg.addPoint(this->stroke->getX(), this->stroke->getY());
        rg.addPoint(this->stroke->getX() + this->stroke->getElementWidth(),
                    this->stroke->getY() + this->stroke->getElementHeight());
    }
    Range repaintRange = rg.unite(this->lastRepaintRange);
    this->lastRepaintRange = rg;

    viewPool->dispatch(xoj::view::SetsquareView::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                       this->getMatrix());
    viewPool->dispatch(xoj::view::SetsquareView::FLAG_DIRTY_REGION, repaintRange);
    handlerPool->dispatch(SetsquareInputHandler::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                          this->getTranslationX(), this->getTranslationY());
}
