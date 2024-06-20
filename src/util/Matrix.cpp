#include "util/Matrix.h"

#include <cmath>
#include <ostream>

namespace xoj::util {


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

auto operator<<(std::ostream& os, Matrix const& m) -> std::ostream& {
    return os << "Matrix(" << m.a << ", " << m.b << ", " << m.tx << "\n"
              << "       " << m.c << ", " << m.d << ", " << m.ty << ")";
}

constexpr auto test_ident = Matrix{1, 0, 0, 1, 0, 0};
constexpr auto test_trans = Matrix{1, 0, 0, 1, 10, 10};
constexpr auto test_scale = Matrix{2, 0, 0, 2, 0, 0};
constexpr auto test_rot = Matrix{0, -1, 1, 0, 0, 0};
constexpr auto test_all = Matrix{2, -1, 1, 2, 10, 10};

static_assert(test_ident * test_ident == test_ident);
static_assert(test_ident * test_trans == test_trans);
static_assert(test_trans * test_ident == test_trans);
static_assert(test_rot.inverse() * test_rot * test_all == test_all);

}  // namespace xoj::util
