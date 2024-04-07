#pragma once

namespace xoj::util::v1 {

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
    // [[nodiscard]] constexpr auto operator*(Point<double> const&) const -> Point<double>;
    [[nodiscard]] auto operator+(Matrix const& other) const -> Matrix;

    double a = 1.0;   ///< x scale
    double b = 0.0;   ///< y skew
    double c = 0.0;   ///< x skew
    double d = 1.0;   ///< y scale
    double tx = 0.0;  ///< x translation
    double ty = 0.0;  ///< y translation
};

auto operator*(double scalar, const Matrix& matrix) -> Matrix;

constexpr auto test_ident = Matrix{1, 0, 0, 1, 0, 0};
constexpr auto test_trans = Matrix{1, 0, 0, 1, 10, 10};
constexpr auto test_scale = Matrix{2, 0, 0, 2, 0, 0};
constexpr auto test_rot = Matrix{0, -1, 1, 0, 0, 0};
constexpr auto test_all = Matrix{2, -1, 1, 2, 10, 10};

} // namespace xoj::util::v1