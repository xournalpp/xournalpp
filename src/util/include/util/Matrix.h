#pragma once

#include <iosfwd>

#if __cplusplus < 202002L
#define CXX20_CONSTEXPR inline
#else
#define CXX20_CONSTEXPR constexpr
#endif

namespace xoj::util {

/**
 * A 2D matrix for affine transformations.
 * a b tx
 * c d ty
 * 0 0  1
 */
struct Matrix {

    [[nodiscard]] constexpr auto operator==(Matrix const& other) const -> bool {
        return a == other.a && b == other.b && c == other.c && d == other.d && tx == other.tx && ty == other.ty;
    }
    [[nodiscard]] constexpr auto operator!=(Matrix const& other) const -> bool { return !(*this == other); }

    // cant be constexpr because of std::cos and std::sin till c++23
    [[nodiscard]] auto rotate(double angle) const -> Matrix;

    [[nodiscard]] constexpr auto scale(double sx, double sy) const -> Matrix {
        return {sx * this->a, sx * this->b, sy * this->c, sy * this->d, sx * this->tx, sy * this->ty};
    }

    [[nodiscard]] constexpr auto translate(double dx, double dy) const -> Matrix {
        return {this->a, this->b, this->c, this->d, tx + this->tx, ty + this->ty};
    }

    [[nodiscard]] constexpr auto inverse() const -> Matrix {
        auto det = this->a * this->d - this->b * this->c;  // 3
        // assert(det != 0);
        return {this->d / det,
                (-this->b) / det,
                (-this->c) / det,
                this->a / det,
                (this->c * this->ty - this->d * this->tx) / det,
                (this->b * this->tx - this->a * this->ty) / det};
    }

    [[nodiscard]] constexpr auto operator*(Matrix const& rhs) const -> Matrix{
            return {this->a * rhs.a + this->b * rhs.c,
            this->a * rhs.b + this->b * rhs.d,
            this->c * rhs.a + this->d * rhs.c,
            this->c * rhs.b + this->d * rhs.d,
            this->a * rhs.tx + this->b * rhs.ty + this->tx,
            this->c * rhs.tx + this->d * rhs.ty + this->ty};
    }

    constexpr auto operator*=(Matrix const& rhs) -> Matrix& {
        *this = (*this) * rhs;
        return *this;
    }

    double a = 1.0;   ///< x scale
    double b = 0.0;   ///< y skew
    double c = 0.0;   ///< x skew
    double d = 1.0;   ///< y scale
    double tx = 0.0;  ///< x translation
    double ty = 0.0;  ///< y translation
};

auto operator<<(std::ostream& os, Matrix const& m) -> std::ostream&;

}  // namespace xoj::util

#undef CXX20_CONSTEXPR
