#pragma once

#include <cmath>
#include <iosfwd>

#include "Point.h"

struct _cairo;

namespace xoj::util {
/**
 * A 2D matrix for affine transformations.
 * xx xy shift.x
 * yx yy shift.y
 *  0  0  1
 */
struct Matrix {
    [[nodiscard]] constexpr auto operator==(Matrix const& other) const -> bool {
        return xx == other.xx && xy == other.xy && yx == other.yx && yy == other.yy && shift.x == other.shift.x &&
               shift.y == other.shift.y;
    }
    [[nodiscard]] constexpr auto operator!=(Matrix const& other) const -> bool { return !(*this == other); }

    [[nodiscard]] static constexpr auto TRANSLATION(double dx, double dy) -> Matrix {
        return Matrix{1, 0, 0, 1, {dx, dy}};
    }
    [[nodiscard]] static constexpr auto SCALING(double mx, double my) -> Matrix { return Matrix{mx, 0, 0, my, {0, 0}}; }

    // todo(cpp23): constexpr
    [[nodiscard]] static inline auto ROTATION(double angle) -> Matrix {
        const double cos = std::cos(angle);
        const double sin = std::sin(angle);
        return Matrix{cos, sin, -sin, cos, {0, 0}};
    }

    // todo(cpp23): constexpr
    [[nodiscard]] inline auto rotate(double angle) const -> Matrix { return *this * ROTATION(angle); }

    [[nodiscard]] constexpr auto scale(double sx, double sy) const -> Matrix { return *this * SCALING(sx, sy); }

    [[nodiscard]] constexpr auto translate(double dx, double dy) const -> Matrix { return *this * TRANSLATION(dx, dy); }

    [[nodiscard]] constexpr auto inverse() const -> Matrix {
        auto det = this->xx * this->yy - this->xy * this->yx;
        // assert(det != 0);
        return {this->yy / det,
                (-this->yx) / det,
                (-this->xy) / det,
                this->xx / det,
                {(this->xy * this->shift.y - this->yy * this->shift.x) / det,
                 (this->yx * this->shift.x - this->xx * this->shift.y) / det}};
    }

    [[nodiscard]] constexpr auto operator*(Matrix const& rhs) const -> Matrix {
        return {this->xx * rhs.xx + this->xy * rhs.yx,
                this->yx * rhs.xx + this->yy * rhs.yx,
                this->xx * rhs.xy + this->xy * rhs.yy,
                this->yx * rhs.xy + this->yy * rhs.yy,
                {this->xx * rhs.shift.x + this->xy * rhs.shift.y + this->shift.x,
                 this->yx * rhs.shift.x + this->yy * rhs.shift.y + this->shift.y}};
    }

    constexpr auto operator*=(Matrix const& rhs) -> Matrix& {
        *this = (*this) * rhs;
        return *this;
    }

    /// Apply the matrix without the offset
    [[nodiscard]] constexpr xoj::util::Point<double> applyToVector(const xoj::util::Point<double>& v) const {
        return {xx * v.x + xy * v.y, yx * v.x + yy * v.y};
    }
    /// Apply the matrix with the offset
    [[nodiscard]] constexpr auto operator*(Point<double> const& pt) const -> Point<double> {
        return applyToVector(pt) + shift;
    }

    /// Our matrices will regularly be fed to cairo
    void transformCairo(_cairo* cr) const;

    // WARNING: do not change the order or you will break toCairo() - see also Matrix.cpp and MatrixTest.cpp
    double xx = 1.0;      ///< x scale
    double yx = 0.0;      ///< x skew
    double xy = 0.0;      ///< y skew
    double yy = 1.0;      ///< y scale
    Point<double> shift;  ///< translation
};
}  // namespace xoj::util

auto operator<<(std::ostream& os, xoj::util::Matrix const& m) -> std::ostream&;
