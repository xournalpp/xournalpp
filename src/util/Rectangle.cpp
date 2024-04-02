#include "util/Rectangle.h"

#include <iostream>

#include "util/Matrix.h"
#include "util/detail/MatrixMultiply.h"

namespace xoj::util {

auto operator*(const Matrix& matrix, const Rectangle<double>& rect) -> Rectangle<double> {
    return multiply(matrix, rect);
}

constexpr auto multiply2(const Matrix& matrix, const Rectangle<double>& rect) -> Rectangle<double> {
    auto p1 = multiply(matrix, Point<double>{rect.x, rect.y});
    auto p2 = multiply(matrix, Point<double>{rect.x + rect.width, rect.y + rect.height});
    auto p3 = multiply(matrix, Point<double>{rect.x, rect.y + rect.height});
    auto p4 = multiply(matrix, Point<double>{rect.x + rect.width, rect.y});
    auto x = std::min({p1.x, p2.x, p3.x, p4.x});
    auto y = std::min({p1.y, p2.y, p3.y, p4.y});
    auto w = std::max({p1.x, p2.x, p3.x, p4.x}) - x;
    auto h = std::max({p1.y, p2.y, p3.y, p4.y}) - y;
    return {x, y, w, h};
}

constexpr auto r1 = Rectangle<double>{1, 3, 4, 6};
constexpr auto m1 = Matrix{0.830704, -0.556714, -0.556714, 0.830704, 2, 3};

auto operator<<(std::ostream& os, Rectangle<double> const& r) -> std::ostream& {
    return os << "Rectangle(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
}

static_assert(multiply(m1, r1) == multiply2(m1, r1));
}  // namespace xoj::util