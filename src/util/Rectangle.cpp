#include "util/Rectangle.h"

#include <iostream>

namespace xoj::util {

// cxx20:
// constexpr auto r1 = Rectangle<double>{1, 3, 4, 6};
// constexpr auto m1 = Matrix{0.830704, -0.556714, -0.556714, 0.830704, 2, 3};

auto operator<<(std::ostream& os, Rectangle<double> const& r) -> std::ostream& {
    return os << "Rectangle(" << r.x << ", " << r.y << ", " << r.width << ", " << r.height << ")";
}

// cxx20: static_assert(m1.inverse() * (m1 * r1) == r1);
}  // namespace xoj::util
