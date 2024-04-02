#pragma once

#include "util/Matrix.h"
#include "util/Point.h"
#include "util/Rectangle.h"


namespace xoj::util {
inline namespace detail {

struct Size {
    double width;
    double height;
};

constexpr auto multiply(const Matrix& matrix, const Size& size) -> Size {
    return {matrix.a * size.width + matrix.b * size.height, matrix.c * size.width + matrix.d * size.height};
}

constexpr auto multiply(const Matrix& matrix, const Point<double>& pt) -> Point<double> {
    return {matrix.a * pt.x + matrix.b * pt.y + matrix.tx, matrix.c * pt.x + matrix.d * pt.y + matrix.ty};
}

constexpr auto multiply(const Matrix& matrix, const Rectangle<double>& rect) -> Rectangle<double> {
    util::Point<double> p1{rect.x, rect.y};
    util::Point<double> p2{rect.x + rect.width, rect.y + rect.height};
    util::Point<double> p3{rect.x, rect.y + rect.height};
    util::Point<double> p4{rect.x + rect.width, rect.y};
    auto s1 = multiply(matrix, p1);
    auto s2 = multiply(matrix, p2);
    auto s3 = multiply(matrix, p3);
    auto s4 = multiply(matrix, p4);
    auto x = std::min({s1.x, s2.x, s3.x, s4.x});
    auto y = std::min({s1.y, s2.y, s3.y, s4.y});
    auto w = std::max({s1.x, s2.x, s3.x, s4.x}) - x;
    auto h = std::max({s1.y, s2.y, s3.y, s4.y}) - y;
    return {x, y, w, h};
}

}  // namespace detail
}  // namespace xoj::util