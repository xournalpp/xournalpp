#include "util/Matrix.h"

#include <cassert>
#include <cmath>
#include <ostream>

namespace xoj::util {

auto constexpr multiply(Matrix const& lhs, Matrix const& rhs) -> Matrix {
    return {lhs.a * rhs.a + lhs.b * rhs.c,
            lhs.a * rhs.b + lhs.b * rhs.d,
            lhs.c * rhs.a + lhs.d * rhs.c,
            lhs.c * rhs.b + lhs.d * rhs.d,
            lhs.a * rhs.tx + lhs.b * rhs.ty + lhs.tx,
            lhs.c * rhs.tx + lhs.d * rhs.ty + lhs.ty};
}

auto constexpr scale(double sx, double sy, Matrix const& rhs) -> Matrix {
    return {sx * rhs.a, sx * rhs.b, sy * rhs.c, sy * rhs.d, sx * rhs.tx, sy * rhs.ty};
}

auto constexpr translate(double tx, double ty, Matrix const& rhs) -> Matrix {
    return {rhs.a, rhs.b, rhs.c, rhs.d, tx + rhs.tx, ty + rhs.ty};
}

auto constexpr inverse(Matrix const& self) -> Matrix {
    auto det = self.a * self.d - self.b * self.c;  // 3
    assert(det != 0);
    return {self.d / det,
            (-self.b) / det,
            (-self.c) / det,
            self.a / det,
            (self.c * self.ty - self.d * self.tx) / det,
            (self.b * self.tx - self.a * self.ty) / det};
}

/// @brief Create a new matrix with \n
///  cos(angle) -sin(angle) 0 \n
/// +sin(angle)  cos(angle) 0  * self \n
///  0           0          1 \n
/// @param angle in radians
auto Matrix::rotate(double angle) const -> Matrix {
    // rotate * transform:
    double cos = std::cos(angle);
    double sin = std::sin(angle);
    double nsin = -sin;
    return {cos * a + nsin * c,   cos * b + nsin * d,  //
            sin * a + cos * c,    sin * b + cos * d,   //
            cos * tx + nsin * ty, sin * tx + cos * ty};
}

auto Matrix::scale(double sx, double sy) const -> Matrix { return xoj::util::scale(sx, sy, *this); }

auto Matrix::translate(double dx, double dy) const -> Matrix { return xoj::util::translate(dx, dy, *this); }

auto Matrix::inverse() const -> Matrix { return xoj::util::inverse(*this); }

auto Matrix::operator*(Matrix const& other) const -> Matrix { return multiply(*this, other); }

auto Matrix::operator*(Point<double> const& pt) const -> Point<double> {
    return {a * pt.x + b * pt.y + tx, c * pt.x + d * pt.y + ty};
}

auto Matrix::operator+(Matrix const& other) const -> Matrix {
    return {a + other.a, b + other.b, c + other.c, d + other.d, tx + other.tx, ty + other.ty};
}

auto operator<<(std::ostream& os, Matrix const& m) -> std::ostream& {
    return os << "Matrix(" << m.a << ", " << m.b << ", " << m.tx << "\n"
              << "       " << m.c << ", " << m.d << ", " << m.ty << ")";
}

constexpr auto test_ident = Matrix{1, 0, 0, 1, 0, 0};
constexpr auto test_trans = Matrix{1, 0, 0, 1, 10, 10};
constexpr auto test_scale = Matrix{2, 0, 0, 2, 0, 0};
constexpr auto test_rot = Matrix{0, -1, 1, 0, 0, 0};
constexpr auto test_all = Matrix{2, -1, 1, 2, 10, 10};

static_assert(multiply(test_ident, test_ident) == test_ident);
static_assert(multiply(test_ident, test_trans) == test_trans);
static_assert(multiply(test_trans, test_ident) == test_trans);
static_assert(multiply(inverse(test_rot), multiply(test_rot, test_all)) == test_all);

}  // namespace xoj::util