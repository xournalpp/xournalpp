#pragma once

#include <iosfwd>

#include "Point.h"


namespace xoj::util {

/**
 * A 2D matrix
 * a b tx
 * c d ty
 * 0 0  1
 */
struct Matrix {
    [[nodiscard]] constexpr auto operator==(Matrix const& other) const -> bool {
        return a == other.a && b == other.b && c == other.c && d == other.d && tx == other.tx && ty == other.ty;
    }
    [[nodiscard]] constexpr auto operator!=(Matrix const& other) const -> bool { return !(*this == other); }

    [[nodiscard]] auto rotate(double angle) const -> Matrix;
    [[nodiscard]] auto scale(double sx, double sy) const -> Matrix;
    [[nodiscard]] auto translate(double dx, double dy) const -> Matrix;
    [[nodiscard]] auto inverse() const -> Matrix;

    [[nodiscard]] auto operator*(Matrix const& other) const -> Matrix;
    [[nodiscard]] auto operator*(Point<double> const&) const -> Point<double>;
    [[nodiscard]] auto operator+(Matrix const& other) const -> Matrix;

    double a = 1.0;   ///< x scale
    double b = 0.0;   ///< y skew
    double c = 0.0;   ///< x skew
    double d = 1.0;   ///< y scale
    double tx = 0.0;  ///< x translation
    double ty = 0.0;  ///< y translation
};

auto operator*(double scalar, const Matrix& matrix) -> Matrix;
auto operator<<(std::ostream& os, Matrix const& m) -> std::ostream&;

}  // namespace xoj::util
