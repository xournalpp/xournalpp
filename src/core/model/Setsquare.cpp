#include "Setsquare.h"

// parameters used when initially displaying setsquare on a page
constexpr double INITIAL_HEIGHT = 8.0;
constexpr double INITIAL_X = 21. * HALF_CM;
constexpr double INITIAL_Y = 15. * HALF_CM;

Setsquare::Setsquare(): height(INITIAL_HEIGHT), rotation(.0), translationX(INITIAL_X), translationY(INITIAL_Y) {}

Setsquare::Setsquare(double height, double rotation, double x, double y):
        height(height), rotation(rotation), translationX(x), translationY(y) {}

Setsquare::~Setsquare() {}

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
