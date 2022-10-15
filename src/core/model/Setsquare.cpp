#include "Setsquare.h"

#include <cmath>             // for sin, cos
#include <initializer_list>  // for initializer_list

#include "gui/inputdevices/SetsquareInputHandler.h"
#include "view/SetsquareView.h"

Setsquare::Setsquare(): Setsquare(INITIAL_HEIGHT, .0, INITIAL_X, INITIAL_Y) {}

Setsquare::Setsquare(double height, double rotation, double x, double y):
        height(height),
        rotation(rotation),
        translationX(x),
        translationY(y),
        viewPool(std::make_shared<xoj::util::DispatchPool<xoj::view::SetsquareView>>()),
        handlerPool(std::make_shared<xoj::util::DispatchPool<SetsquareInputHandler>>()) {}

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
        rg.addPadding(0.5 * this->stroke->getWidth());
    }
    Range repaintRange = rg.unite(this->lastRepaintRange);
    this->lastRepaintRange = rg;

    cairo_matrix_t matrix{};
    this->getMatrix(matrix);
    viewPool->dispatch(xoj::view::SetsquareView::UPDATE_VALUES, this->getHeight(), this->getRotation(), matrix);
    viewPool->dispatch(xoj::view::SetsquareView::FLAG_DIRTY_REGION, repaintRange);
    handlerPool->dispatch(SetsquareInputHandler::UPDATE_VALUES, this->getHeight(), this->getRotation(),
                          this->getTranslationX(), this->getTranslationY());
}

void Setsquare::setHeight(double height) { this->height = height; }
auto Setsquare::getHeight() const -> double { return this->height; }

void Setsquare::setRotation(double rotation) { this->rotation = rotation; }
auto Setsquare::getRotation() const -> double { return this->rotation; }

void Setsquare::setTranslationX(double x) { this->translationX = x; }
auto Setsquare::getTranslationX() const -> double { return this->translationX; }

void Setsquare::setTranslationY(double y) { this->translationY = y; }
auto Setsquare::getTranslationY() const -> double { return this->translationY; }

void Setsquare::getMatrix(cairo_matrix_t& matrix) const {
    cairo_matrix_init_identity(&matrix);
    cairo_matrix_translate(&matrix, this->translationX, this->translationY);
    cairo_matrix_rotate(&matrix, this->rotation);
    cairo_matrix_scale(&matrix, CM, CM);
}

auto Setsquare::getViewPool() const -> const std::shared_ptr<xoj::util::DispatchPool<xoj::view::SetsquareView>>& {
    return viewPool;
}
auto Setsquare::getHandlerPool() const -> const std::shared_ptr<xoj::util::DispatchPool<SetsquareInputHandler>>& {
    return handlerPool;
}

auto Setsquare::getStroke() const -> Stroke* { return this->stroke; }

void Setsquare::setStroke(Stroke* s) { this->stroke = s; }
