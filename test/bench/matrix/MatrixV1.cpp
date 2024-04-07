

#include <cmath>
#include "MatrixV1.h"
#include "MatrixConst.h"

namespace xoj::util::v1 {

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

auto Matrix::scale(double sx, double sy) const -> Matrix { return ::scale(sx, sy, *this); }

auto Matrix::translate(double dx, double dy) const -> Matrix { return ::translate(dx, dy, *this); }

auto Matrix::inverse() const -> Matrix { return ::inverse(*this); }

auto Matrix::operator*(Matrix const& other) const -> Matrix { return multiply(*this, other); }

// auto Matrix::operator*(Point<double> const& pt) const -> Point<double> {
//     return {a * pt.x + b * pt.y + tx, c * pt.x + d * pt.y + ty};
// }

auto Matrix::operator+(Matrix const& other) const -> Matrix {
    return {a + other.a, b + other.b, c + other.c, d + other.d, tx + other.tx, ty + other.ty};
}

static_assert(multiply(test_ident, test_ident) == test_ident);
static_assert(multiply(test_ident, test_trans) == test_trans);
static_assert(multiply(test_trans, test_ident) == test_trans);
static_assert(multiply(inverse(test_rot), multiply(test_rot, test_all)) == test_all);

}  // namespace xoj::util::v1