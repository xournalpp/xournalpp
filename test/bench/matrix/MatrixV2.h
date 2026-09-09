#pragma once

#include <cmath>

#include "MatrixConst.h"


namespace xoj::util::v2 {

struct Matrix {
    [[nodiscard]] constexpr auto operator==(Matrix const& other) const -> bool {
        return a == other.a && b == other.b && c == other.c && d == other.d && tx == other.tx && ty == other.ty;
    }
    [[nodiscard]] constexpr auto operator!=(Matrix const& other) const -> bool { return !(*this == other); }

    [[nodiscard]] auto rotate(double angle) const -> Matrix {
        // rotate * transform:
        double cos = std::cos(angle);
        double sin = std::sin(angle);
        double nsin = -sin;
        return {cos * a + nsin * c,   cos * b + nsin * d,  //
                sin * a + cos * c,    sin * b + cos * d,   //
                cos * tx + nsin * ty, sin * tx + cos * ty};
    }

    [[nodiscard]] constexpr auto scale(double sx, double sy) const -> Matrix { return ::scale(sx, sy, *this); }

    [[nodiscard]] constexpr auto translate(double dx, double dy) const -> Matrix { return ::translate(dx, dy, *this); }

    [[nodiscard]] constexpr auto inverse() const -> Matrix { return ::inverse(*this); }

    [[nodiscard]] constexpr auto operator*(Matrix const& other) const -> Matrix { return multiply(*this, other); }

    // [[nodiscard]] constexpr auto operator*(Point<double> const&) const -> Point<double> {
    //     return {a * pt.x + b * pt.y + tx, c * pt.x + d * pt.y + ty};
    // }

    [[nodiscard]] constexpr auto operator+(Matrix const& other) const -> Matrix {
        return {a + other.a, b + other.b, c + other.c, d + other.d, tx + other.tx, ty + other.ty};
    }

    double a = 1.0;   ///< x scale
    double b = 0.0;   ///< y skew
    double c = 0.0;   ///< x skew
    double d = 1.0;   ///< y scale
    double tx = 0.0;  ///< x translation
    double ty = 0.0;  ///< y translation
};

constexpr auto test_ident = Matrix{1, 0, 0, 1, 0, 0};
constexpr auto test_trans = Matrix{1, 0, 0, 1, 10, 10};
constexpr auto test_scale = Matrix{2, 0, 0, 2, 0, 0};
constexpr auto test_rot = Matrix{0, -1, 1, 0, 0, 0};
constexpr auto test_all = Matrix{2, -1, 1, 2, 10, 10};

static_assert(multiply(test_ident, test_ident) == test_ident);
static_assert(multiply(test_ident, test_trans) == test_trans);
static_assert(multiply(test_trans, test_ident) == test_trans);
static_assert(multiply(inverse(test_rot), multiply(test_rot, test_all)) == test_all);

}  // namespace xoj::util::v2
