#pragma once

#include <algorithm>

#include "util/Matrix.h"
#include "util/Rectangle.h"

#include "PointMultiply.h"


namespace xoj::util {

constexpr auto operator*(const Matrix& matrix, const Rectangle<double>& rect) -> Rectangle<double> {
    util::Point<double> p1{rect.x, rect.y};
    util::Point<double> p2{rect.x + rect.width, rect.y + rect.height};
    util::Point<double> p3{rect.x, rect.y + rect.height};
    util::Point<double> p4{rect.x + rect.width, rect.y};
    auto s1 = matrix * p1;
    auto s2 = matrix * p2;
    auto s3 = matrix * p3;
    auto s4 = matrix * p4;
    auto x = std::minmax({s1.x, s2.x, s3.x, s4.x});
    auto y = std::minmax({s1.y, s2.y, s3.y, s4.y});
    return {x.first, y.first, x.second, y.second};
}

}  // namespace xoj::util
