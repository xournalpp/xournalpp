#pragma once

#include "util/Matrix.h"
#include "util/Point.h"

namespace xoj::util {

[[nodiscard]] constexpr auto operator*(Matrix const& lhs, Point<double> const& pt) -> Point<double> {
    return {lhs.a * pt.x + lhs.b * pt.y + lhs.tx, lhs.c * pt.x + lhs.d * pt.y + lhs.ty};
}

}  // namespace xoj::util
